#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <tnl.h>
#include <modules/texman/TextureManager.h>
#include <modules/math/Transform.h>

class Model : virtual public ::Object {
    Model();
    Model(const Model &);
public:
    struct Corner {
        int v, n, t;
    };
    
    typedef std::vector<Corner> Face;

    struct Material;
    struct Group;
    struct MeshData;
    class Object;

    Model(TextureManager & texman, const std::string & filename);
    virtual ~Model();

    void draw(JRenderer &, const Matrix & Mmodel, const Matrix & Mnormal);
    Ptr<Object> getObject(const std::string & name);
    inline Ptr<Object> getDefaultObject() { return objects.front(); }
    
    /// Sets the cull mode for all contained objects
    void setCullmode(jrcullmode_t);
    
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
    typedef std::vector<Face> Faces;

    std::string name;
    Material mtl;
    Faces faces;
    jrcullmode_t cullmode;

    inline Group(const std::string & name = "" )
    : name(name), cullmode(JR_CULLMODE_CULL_NEGATIVE) { }
};

struct Model::MeshData : public ::Object {
    typedef std::vector<Vector> Vectors;
    Vectors vertices, normals, texcoords;
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
    
    /// Sets the cull mode for all contained groups
    void setCullmode(jrcullmode_t);
    
    inline Ptr<MeshData> getMeshData() const { return meshdata; }
    
    const std::vector<Ptr<Group> > & getGroups() const { return groups; }
};

#endif
