#include <fstream>
#include <string>
#include <cstring>
#include <GL/glew.h>
#include <GL/gl.h>
#include <IL/il.h>
#include <IL/ilut.h>

#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <modules/jogi/JRenderer.h>

#include <tnl.h>

#include "Water.h"

class WaterImpl : public Object
{
    JRenderer *r;
    float tile_size, tile_uvspan;
    GLuint tex, bump_tex;
    Ptr<IConfig> cfg;
    Ptr<IGame> thegame;
    bool all_ok;
    GLhandleARB vertex_shader, fragment_shader, program;
    // number of tiles to draw in each dimension
    int tiles_num;

public:
    WaterImpl(Ptr<IGame> game) {
        thegame = game;
        r = game->getRenderer();
        cfg = game->getConfig();
        tile_size = cfg->queryFloat("Water_tile_size", 1000.0f);
        tile_uvspan = cfg->queryFloat("Water_tile_uvspan", 1.0f);
        tiles_num = cfg->queryInt("Water_tile_num", 21);

        all_ok = true;
        
        const char *extensions = "GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader";
        if (glewIsSupported(extensions)) {
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                cfg->queryInt("Game_mirror_texture_size", 512),
                cfg->queryInt("Game_mirror_texture_size", 512),
                0,
                GL_RGB, GL_BYTE, NULL);
                
            ls_message("Compiling vertex shader.\n");
            vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
            readShader(vertex_shader, cfg->query("Water_vertex_shader"));
            compileShader(vertex_shader);
            
            ls_message("Compiling fragment shader.\n");
            fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
            readShader(fragment_shader, cfg->query("Water_fragment_shader"));
            compileShader(fragment_shader);
            
            program = glCreateProgramObjectARB();
            linkProgram(program, vertex_shader, fragment_shader);
            glDeleteObjectARB(vertex_shader);
            glDeleteObjectARB(fragment_shader);

            {
                glValidateProgramARB(program);
                GLcharARB buf[1024];
                glGetInfoLogARB(program, 1024, NULL, buf);
                ls_message("Validation result: %s\n", buf);
            }
            
            char buf[256];
            strcpy(buf, cfg->query("Water_bumpmap"));
            bump_tex = ilutGLLoadImage(buf);
        } else {
            ls_warning("OpenGL Shading Language extensions not supported. Water will be ugly.\n");
            all_ok = false;
        }
    }
    
    ~WaterImpl() {
        if (all_ok) {
            glDeleteTextures(1, &tex);
            glDeleteTextures(1, &bump_tex);
            glDeleteObjectARB(program);
        }
    }
    
    void draw() {
        float clip_far = r->getClipFar();
        
        Vector campos = thegame->getCamera()->getLocation();
        

        int tile_x_begin = int(std::floor(campos[0] / tile_size) - tiles_num/2);
        int tile_z_begin = int(std::floor(campos[2] / tile_size) - tiles_num/2);
        int tile_x_end = tile_x_begin + tiles_num;
        int tile_z_end = tile_z_begin + tiles_num;
        
        glActiveTexture(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, tex);

        glActiveTexture(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_2D, bump_tex);
        glEnable(GL_TEXTURE_2D);
        
        glUseProgramObjectARB(program);

        glUniform1fARB(glGetUniformLocationARB(program, "waves"), 0.778f);
        glUniform1fARB(glGetUniformLocationARB(program, "aspect"), r->getAspect());
        glUniform1fARB(glGetUniformLocationARB(program, "focus"), thegame->getCamera()->getFocus());
        glUniform3fARB(glGetUniformLocationARB(program, "BaseColor"), 0.23, 0.421, 0.418);
        glUniform1fARB(glGetUniformLocationARB(program, "FresnelExponent"), 1.702f);
        glUniform1fARB(glGetUniformLocationARB(program, "Reflectivity"), 0.943f);
        glUniform1iARB(glGetUniformLocationARB(program, "MirrorMap"), 0);
        glUniform1iARB(glGetUniformLocationARB(program, "BumpMap"), 1);
        
        glUniform3fARB(glGetUniformLocationARB(program, "CamPos"), campos[0], campos[1], campos[2]);

        glBegin(GL_QUADS);
        glNormal3f(0,1.0f,0);
        glColor4f(1,1,1,1);
        for (int tile_z=tile_z_begin; tile_z < tile_z_end; ++tile_z) {
            for (int tile_x=tile_x_begin; tile_x < tile_x_end; ++tile_x) {
                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);
                glVertex3f(tile_size * tile_x, 0, tile_size * tile_z);

                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, tile_uvspan, 0);
                glVertex3f(tile_size * (tile_x+1), 0, tile_size * tile_z);

                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, tile_uvspan, tile_uvspan);
                glVertex3f(tile_size * (tile_x+1), 0, tile_size * (tile_z+1));

                glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, tile_uvspan);
                glVertex3f(tile_size * tile_x, 0, tile_size * (tile_z+1));
            }
        }
        glEnd();

        glActiveTexture(GL_TEXTURE1_ARB);
        glDisable(GL_TEXTURE_2D);

        glUseProgramObjectARB(0);
        glActiveTexture(GL_TEXTURE0_ARB);
    }
    
    void copyTex() {
        int sz = cfg->queryInt("Game_mirror_texture_size", 512);

        glReadBuffer(GL_BACK);
        glBindTexture(GL_TEXTURE_2D, tex);
        glCopyTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGB,
            0,0,sz,sz,
            0);
    }
    
    bool supportsMirrorTex() {
        return all_ok;
    }
    
    void readShader(GLhandleARB shader, const char *filename) {
        std::ifstream in(filename);
        if (!in) {
            ls_error("Error reading shader: %s\n", filename);
        }
        in.seekg(0, std::ios::end);
        int length = in.tellg();
        in.seekg(0, std::ios::beg);
        
        GLcharARB *buffer = new char[length+1];
        in.read(buffer, length);
        buffer[length] = 0;
        
        const GLcharARB *strings[] = {buffer};
        glShaderSourceARB(shader, 1, strings, NULL);
        ls_message("Shader source:\n%s\n", buffer);
        delete [] buffer;
    }
    
    bool compileShader(GLhandleARB shader) {
        GLint status;
        
        glCompileShaderARB(shader);
        glGetObjectParameterivARB(shader, GL_OBJECT_COMPILE_STATUS_ARB, &status);
        GLcharARB buf[1024];
        glGetInfoLogARB(shader, 1024, NULL, buf);
        if (GL_TRUE == status) {
            ls_message("Compiling successful: %s\n", buf);
            return true;
        } else {
            ls_error("Error compiling shader: %s\n", buf);
            return false;
        }
    }
    
    bool linkProgram(GLhandleARB prg, GLhandleARB vshader, GLhandleARB fshader) {
        glAttachObjectARB(prg, vshader);
        glAttachObjectARB(prg, fshader);
        glLinkProgram(prg);

        GLint status;
        glGetObjectParameterivARB(prg, GL_OBJECT_LINK_STATUS_ARB, &status);
        GLcharARB buf[1024];
        glGetInfoLogARB(prg, 1024, NULL, buf);
        if (GL_TRUE == status) {
            ls_message("Linking successful: %s\n", buf);
            return true;
        } else {
            ls_error("Error linking program: %s\n", buf);
            return false;
        }
    }

}; // class WaterImpl

Water::Water(Ptr<IGame> game) {
    pImpl = new WaterImpl(game);
}

Water::~Water() { }

void Water::draw() {
    pImpl->draw();
}

void Water::copyTex() {
    pImpl->copyTex();
}

bool Water::supportsMirrorTex() {
    return pImpl->supportsMirrorTex();
}

