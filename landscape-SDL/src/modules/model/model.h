#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <list>
#include <landscape.h>

class Model : virtual public Object {
    struct Material;
    struct Face;
    class Group;

    inline Model();              // defined below
    inline Model(const Model &); // defined below
public:
    // defined below
    inline Model(TextureManager & texman, std::istream & in, const char *dir);

    void draw(JRenderer &, const Matrix & Mmodel, const Matrix & Mnormal);
    
    void debugTextures(JRenderer & r, const Matrix & M);
    void dump();
    
protected:
    void parseObjFile(TextureManager & texman, std::istream &, const char *dir);
    void parseMtlFile(TextureManager & texman, std::istream &, const char *dir);

protected:
    std::string path;                 // Directory in which texture and material
                                 // files are expected to be found
    std::map<std::string, Material> mtls;  // Materials referenced by name
    std::vector<Vector> v;            // Vertices
    std::vector<Vector> n;            // Normals
    std::vector<Vector> t;            // UVW coords
    std::list<Group> grps;            // Groups of Material+faces
};

struct Model::Material {
    inline Material() : use_tex(false) { }
    bool use_tex;
    int w,h;
    TexPtr tex;
};

struct Model::Face {
    Face() { for(int i=0; i<3; i++) v[i] = n[i] = t[i] = 0; }
    int v[3], n[3], t[3];
};

class Model::Group {
public:
    Material & mtl;
    std::vector<Face> f;             // faces

    Group( Material & mtl ) : mtl(mtl) { }
};


Model::Model() { }
Model::Model(const Model &) { };
Model::Model(TextureManager & texman, std::istream & in, const char *dir)
: path(dir)
{
    parseObjFile(texman, in, dir);
}


#endif
