#include <fstream>
#include <string>
#include <cstring>
#ifdef HAVE_REGAL
#include <GL/Regal.h>
#else
#include <glew.h>
#include <gl.h>
#endif

#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <modules/camera/SimpleCamera.h>
#include <modules/clock/clock.h>
#include <modules/environment/environment.h>
#include <modules/jogi/JRenderer.h>
#include <modules/jogi/JSprite.h>
#include <modules/texman/TextureManager.h>

#include <SceneRenderPass.h>
#include <tnl.h>

#include "Water.h"

class WaterImpl : public SigObject
{
    JRenderer *r;
    float tile_size, tile_uvspan;
    Ptr<Texture> bump_tex, fallback_tex;
    Ptr<IConfig> cfg;
    Ptr<IGame> thegame;
    bool use_shaders;
    GLuint program;
    // number of tiles to draw in each dimension
    int tiles_num;
    double age;
    bool first_draw;

public:
    WaterImpl(Ptr<IGame> game)
        : first_draw(true)
    {
        age = 0;
        thegame = game;
        r = game->getRenderer();
        cfg = game->getConfig();
        tile_size = cfg->queryFloat("Water_tile_size", 1000.0f);
        tile_uvspan = cfg->queryFloat("Water_tile_uvspan", 1.0f);
        tiles_num = cfg->queryInt("Water_tile_num", 21);

        use_shaders = cfg->queryBool("Game_use_shaders", true) && cfg->queryBool("Water_use_shaders", true);
        #ifdef __EMSCRIPTEN__
        use_shaders = false;
        #endif
        
        if (use_shaders) {
            ls_message("Compiling vertex shader.\n");
            GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
            readShader(vertex_shader, cfg->query("Water_vertex_shader"));
            compileShader(vertex_shader);
            
            ls_message("Compiling fragment shader.\n");
            GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
            readShader(fragment_shader, cfg->query("Water_fragment_shader"));
            compileShader(fragment_shader);
            
            program = glCreateProgram();
            linkProgram(program, vertex_shader, fragment_shader);
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);

            {
                glValidateProgram(program);
                GLchar buf[1024];
                glGetProgramInfoLog(program, 1024, NULL, buf);
                ls_message("Validation result: %s\n", buf);
            }
            
            bump_tex = game->getTexMan()->query(cfg->query("Water_bumpmap"), JR_HINT_FULLOPACITY);
        } else {
            fallback_tex = game->getTexMan()->query(cfg->query("Water_fallback_texture"), JR_HINT_FULLOPACITY);
        }
    }
    
    ~WaterImpl() {
        if (use_shaders) {
            glDeleteProgram(program);
        }
    }
    
    void update() {
        age += thegame->getClock()->getFrameDelta();
    }

    void draw(SceneRenderPass *pass) {
        if (first_draw) ls_message("Water: First draw.\n");
        
        if (use_shaders) {
            drawWithShaders(pass->getMirrorTexture(), pass->getMirrorCamera()->camToMir());
        } else {
            drawWithoutShaders();
        }

        if (first_draw) {
            ls_message("Water: First draw finished.\n");
            first_draw = false;
        }
    }
    
    void drawWithShaders(Ptr<Texture> tex, const Matrix3 cam2mir) {
        Vector campos = thegame->getCamera()->getLocation();
        
        int tile_x_begin = int(std::floor(campos[0] / tile_size) - tiles_num/2);
        int tile_z_begin = int(std::floor(campos[2] / tile_size) - tiles_num/2);
        int tile_x_end = tile_x_begin + tiles_num;
        int tile_z_end = tile_z_begin + tiles_num;
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,
            thegame->getRenderer()->getGLTexFromTxtid(tex->getTxtid()));
        glEnable(GL_TEXTURE_2D);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, r->getGLTexFromTxtid(bump_tex->getTxtid()));
        glEnable(GL_TEXTURE_2D);

        if (first_draw) ls_message("Use program object.\n");
        glUseProgram(program);
        
        if (first_draw) ls_message("Specify uniform values.\n");
        glUniform1f(glGetUniformLocation(program, "waves"), 0.778f);
        glUniform1f(glGetUniformLocation(program, "aspect"), r->getAspect());
        glUniform1f(glGetUniformLocation(program, "focus"), thegame->getCamera()->getFocus());
        glUniform3f(glGetUniformLocation(program, "BaseColor"), 0.23, 0.421, 0.418);
        glUniform1f(glGetUniformLocation(program, "FresnelExponent"), 1.702f);
        glUniform1f(glGetUniformLocation(program, "Reflectivity"), 0.943f);
        glUniform1i(glGetUniformLocation(program, "MirrorMap"), 0);
        glUniform1i(glGetUniformLocation(program, "BumpMap"), 1);
        glUniform1f(glGetUniformLocation(program, "Time"), age);
        glUniform3f(glGetUniformLocation(program, "CamPos"), campos[0], campos[1], campos[2]);
        glUniformMatrix3fv(glGetUniformLocation(program, "CamToMir"), 1, GL_FALSE, cam2mir.glMatrix());

        GLuint vtx_buffer;
        // Generate an ID (buffer object name)
        glGenBuffers(1, &vtx_buffer);
        // Binds that ID to the binding point GL_ARRAY_BUFFER and creates a buffer object
        glBindBuffer(GL_ARRAY_BUFFER, vtx_buffer);

        GLuint elem_buffer;
        glGenBuffers(1, &elem_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vtx_buffer);
        GLfloat elem_data[] = {0, 1, 2, 3};
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elem_data), elem_data, GL_STATIC_DRAW);

        if (first_draw) ls_message("Drawing.\n");
        glBegin(GL_QUADS);
        glNormal3f(0,1.0f,0);
        glColor4f(1,1,1,1);
        for (int tile_z=tile_z_begin; tile_z < tile_z_end; ++tile_z) {
            for (int tile_x=tile_x_begin; tile_x < tile_x_end; ++tile_x) {
                GLfloat vtx_data[] = {
                    tile_size * tile_x, 0, tile_size * tile_z, 0, 0,
                    tile_size * (tile_x+1), 0, tile_size * tile_z, tile_uvspan, 0,
                    tile_size * (tile_x+1), 0, tile_size * (tile_z+1), tile_uvspan, tile_uvspan,
                    tile_size * tile_x, 0, tile_size * (tile_z+1), 0, tile_uvspan,
                };
                /*
                glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_data), vtx_data, GL_STATIC_DRAW);
                glClientActiveTexture(GL_TEXTURE0);
                glTexCoordPointer(2, GL_FLOAT, 12, vtx_data + 3);
                //glEnableTex


                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);
                glVertex3f(tile_size * tile_x, 0, tile_size * tile_z);

                glMultiTexCoord2f(GL_TEXTURE0, tile_uvspan, 0);
                glVertex3f(tile_size * (tile_x+1), 0, tile_size * tile_z);

                glMultiTexCoord2f(GL_TEXTURE0, tile_uvspan, tile_uvspan);
                glVertex3f(tile_size * (tile_x+1), 0, tile_size * (tile_z+1));

                glMultiTexCoord2f(GL_TEXTURE0, 0, tile_uvspan);
                glVertex3f(tile_size * tile_x, 0, tile_size * (tile_z+1));
                */
            }
        }
        glEnd();
        if (first_draw) ls_message("Done drawing.\n");

        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);

        glUseProgram(0);
        glActiveTexture(GL_TEXTURE0);
    }

    void drawWithoutShaders() {
        Vector campos = thegame->getCamera()->getLocation();
        
        int tile_x_begin = int(std::floor(campos[0] / tile_size) - tiles_num/2);
        int tile_z_begin = int(std::floor(campos[2] / tile_size) - tiles_num/2);
        int tile_x_end = tile_x_begin + tiles_num;
        int tile_z_end = tile_z_begin + tiles_num;

        JRenderer *r = thegame->getRenderer();

        r->setTexture(fallback_tex->getTxtid());
        r->enableTexturing();
        r->enableFog();

        r->setColor(Vector(1,1,1));
        for (int tile_z=tile_z_begin; tile_z < tile_z_end; ++tile_z) {
            for (int tile_x=tile_x_begin; tile_x < tile_x_end; ++tile_x) {
                r->begin(JR_DRAWMODE_TRIANGLE_FAN);
                r->setNormal(Vector(0,1.0f,0));
                r->setUVW(Vector(0,0,0));
                r->vertex(Vector(tile_size * tile_x, 0, tile_size * tile_z));

                r->setNormal(Vector(0,1.0f,0));
                r->setUVW(Vector(tile_uvspan, 0, 0));
                r->vertex(Vector(tile_size * (tile_x+1), 0, tile_size * tile_z));

                r->setNormal(Vector(0,1.0f,0));
                r->setUVW(Vector(tile_uvspan, tile_uvspan, 0));
                r->vertex(Vector(tile_size * (tile_x+1), 0, tile_size * (tile_z+1)));

                r->setNormal(Vector(0,1.0f,0));
                r->setUVW(Vector(0, tile_uvspan, 0));
                r->vertex(Vector(tile_size * tile_x, 0, tile_size * (tile_z+1)));
                r->end();
            }
        }
        r->disableFog();
        r->disableTexturing();
        
        Ptr<Environment> env = thegame->getEnvironment();
        r->setZBufferFunc(JR_ZBFUNC_EQUAL);
        r->setColor(env->getFogColor());
        r->enableAlphaBlending();
        r->begin(JR_DRAWMODE_QUADS);
        for (int tile_z=tile_z_begin; tile_z < tile_z_end; ++tile_z) {
            for (int tile_x=tile_x_begin; tile_x < tile_x_end; ++tile_x) {
                Vector v1(tile_size * tile_x, 0, tile_size * tile_z);
                r->setAlpha(env->getGroundFogStrengthAt(v1));
                r->vertex(v1);

                Vector v2(tile_size * (tile_x+1), 0, tile_size * tile_z);
                r->setAlpha(env->getGroundFogStrengthAt(v2));
                r->vertex(v2);

                Vector v3(tile_size * (tile_x+1), 0, tile_size * (tile_z+1));
                r->setAlpha(env->getGroundFogStrengthAt(v3));
                r->vertex(v3);

                Vector v4(tile_size * tile_x, 0, tile_size * (tile_z+1));
                r->setAlpha(env->getGroundFogStrengthAt(v4));
                r->vertex(v4);
            }
        }
        r->end();
        r->setZBufferFunc(JR_ZBFUNC_LEQUAL);
        r->disableAlphaBlending();
    }
    
    void readShader(GLuint shader, const char *filename) {
        std::ifstream in(filename);
        if (!in) {
            ls_error("Error reading shader: %s\n", filename);
        }
        in.seekg(0, std::ios::end);
        int length = in.tellg();
        in.seekg(0, std::ios::beg);
        
        GLchar *buffer = new char[length+1];
        in.read(buffer, length);
        buffer[length] = 0;
        
        const GLchar *strings[] = {buffer};
        glShaderSource(shader, 1, strings, NULL);
        ls_message("Shader source:\n%s\n", buffer);
        delete [] buffer;
    }
    
    bool compileShader(GLuint shader) {
        GLint status;
        
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        GLchar buf[1024];
        glGetShaderInfoLog(shader, 1024, NULL, buf);
        if (GL_TRUE == status) {
            ls_message("Compiling successful: [%s]\n", buf);
            return true;
        } else {
            ls_error("Error compiling shader: [%s]\n", buf);
            return false;
        }
    }
    
    bool linkProgram(GLuint prg, GLuint vshader, GLuint fshader) {
        glAttachShader(prg, vshader);
        glAttachShader(prg, fshader);
        glLinkProgram(prg);

        GLint status;
        glGetShaderiv(prg, GL_LINK_STATUS, &status);
        GLchar buf[1024];
        glGetProgramInfoLog(prg, 1024, NULL, buf);
        if (GL_TRUE == status) {
            ls_message("Linking successful: [%s]\n", buf);
            return true;
        } else {
            ls_error("Error linking program: [%s]\n", buf);
            return false;
        }
    }
    
    bool needsRenderPass() {
        return use_shaders;
    }
}; // class WaterImpl

Water::Water(Ptr<IGame> game) {
    pImpl = new WaterImpl(game);
}

Water::~Water() { }

void Water::draw(SceneRenderPass* pass) {
    pImpl->draw(pass);
}
bool Water::needsRenderPass() {
    return pImpl->needsRenderPass();
}
void Water::update() {
    pImpl->update();
}

