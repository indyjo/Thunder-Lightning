#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <landscape.h>
#include <modules/texman/TextureManager.h>
#include <modules/math/Transform.h>

class Model : virtual public ::Object {
    struct Corner {
        int v, n, t;
    };
    
    typedef std::vector<Corner> Face;

    Model();
    Model(const Model &);
public:
    struct Material;
    struct Group;
    struct MeshData;
    class Object;

    Model(TextureManager & texman, const std::string & filename);
    virtual ~Model();

    void draw(JRenderer &, const Matrix & Mmodel, const Matrix & Mnormal);
    Ptr<Object> getObject(const std::string & name);
    
protected:
    void parseObjFile(TextureManager & texman, const std::string & filename);
    void parseMtlFile(TextureManager & texman, const std::string & filename,
        std::map<std::string, Material> & mtls);

protected:
    std::vector<Ptr<Object> > objects;            // Groups of Material+faces
};

struct Model::Material {
    inline Material() : use_tex(false), Kd(1,0,1), Ka(0,0,0), Ks(0,0,0), Ns(1)
    { }
    bool use_tex;
    int w,h;
    TexPtr tex;
    Vector Kd, Ka, Ks;
    float Ns;
};

struct Model::Group : public ::Object {
    std::string name;
    Material mtl;
    std::vector<Face> faces;

    inline Group(const std::string & name = "" )
    : name(name) { }
};

struct Model::MeshData : public ::Object {
    std::vector<Vector> vertices, normals, texcoords;
};

class Model::Object : public ::Object {
    friend class Model;

    std::string name;
    std::vector<Ptr<Group> > groups;
    Ptr<MeshData> meshdata;
    
public:
    inline Object(Ptr<MeshData> meshdata, const std::string & name="")
    	: meshdata(meshdata)
    	, name(name)
    { }
    
    inline const std::string & getName() { return name; }
    void draw(JRenderer &);
};

#endif
