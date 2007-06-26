#include <fstream>
#include <string>
#include <cstring>
#include <GL/glew.h>
#include <GL/gl.h>
#include <IL/il.h>
#include <IL/ilut.h>

#include <interfaces/ICamera.h>
#include <interfaces/IConfig.h>
#include <modules/clock/clock.h>
#include <modules/jogi/JRenderer.h>

#include <RenderContext.h>
#include <RenderPass.h>
#include <tnl.h>

#include "Water.h"

class WaterImpl : public SigObject
{
    JRenderer *r;
    float tile_size, tile_uvspan;
    GLuint bump_tex;
    Ptr<IConfig> cfg;
    Ptr<IGame> thegame;
    bool all_ok;
    GLhandleARB vertex_shader, fragment_shader, program;
    // number of tiles to draw in each dimension
    int tiles_num;
    double age;
    Ptr<RenderPass> render_pass;

public:
    WaterImpl(Ptr<IGame> game) {
        age = 0;
        thegame = game;
        r = game->getRenderer();
        cfg = game->getConfig();
        tile_size = cfg->queryFloat("Water_tile_size", 1000.0f);
        tile_uvspan = cfg->queryFloat("Water_tile_uvspan", 1.0f);
        tiles_num = cfg->queryInt("Water_tile_num", 21);

        all_ok = true;
        
        const char *extensions = "GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader";
        if (glewIsSupported(extensions)) {
            render_pass = thegame->getRenderPassList()->createRenderPass();
            render_pass->setEnabled(true);
            render_pass->setResolution(
                cfg->queryInt("Game_mirror_texture_size", 512),
                cfg->queryInt("Game_mirror_texture_size", 512));
            render_pass->setRenderToTexture(true);
            render_pass->preScene().connect(
                SigC::slot(*this, &WaterImpl::updateContext));
            
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
            glDeleteTextures(1, &bump_tex);
            glDeleteObjectARB(program);
        }
    }
    
    void updateContext(Ptr<RenderPass>) {
        RenderContext ctx = RenderContext::MirroredRenderContext(thegame->getCamera());
        render_pass->setRenderContext(ctx);
        render_pass->setRenderContextEnabled(true);
    }
    
    void draw() {
        age += thegame->getClock()->getFrameDelta();
        
        if (!render_pass) {
            return;
        }
        
        Vector campos = thegame->getCamera()->getLocation();
        
        int tile_x_begin = int(std::floor(campos[0] / tile_size) - tiles_num/2);
        int tile_z_begin = int(std::floor(campos[2] / tile_size) - tiles_num/2);
        int tile_x_end = tile_x_begin + tiles_num;
        int tile_z_end = tile_z_begin + tiles_num;
        
        glActiveTexture(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, render_pass->getTexture());
        glEnable(GL_TEXTURE_2D);

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
        glUniform1fARB(glGetUniformLocationARB(program, "Time"), age);
        
        glUniform3fARB(glGetUniformLocationARB(program, "CamPos"), campos[0], campos[1], campos[2]);
        
        Matrix3 cam_orient = thegame->getCamera()->getOrient();
        Vector up,right,front;
        
        thegame->getCamera()->getOrientation(&up,&right,&front);
        right[1] = -right[1];
        front[1] = -front[1];
        up = front%right;
        
        Matrix3 mir_orient = MatrixFromColumns(right, up, front);
        Matrix3 cam2mir = mir_orient.transpose() * cam_orient;
        glUniformMatrix3fvARB(glGetUniformLocationARB(program, "CamToMir"), 1, GL_FALSE, cam2mir.glMatrix());

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
            ls_message("Compiling successful: [%s]\n", buf);
            return true;
        } else {
            ls_error("Error compiling shader: [%s]\n", buf);
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
            ls_message("Linking successful: [%s]\n", buf);
            return true;
        } else {
            ls_error("Error linking program: [%s]\n", buf);
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

