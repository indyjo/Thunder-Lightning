#include "modelman.h"
#include <modules/jogi/JRenderer.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "Skeleton.h"

using namespace std;

Bone::Bone(const std::string & name, Bone *parent, const Vector & pivot)
:   parent(parent), name(name), pivot(pivot)
{
    setTransform(Transform(Quaternion(1,0,0,0), Vector(0,0,0)));
}

Bone::Bone(const std::string & name, const Vector & pivot)
:   parent(0), name(name), pivot(pivot)
{
    setTransform(Transform(Quaternion(1,0,0,0), Vector(0,0,0)));
}

Ptr<Bone> Bone::createChild(const std::string & name, const Vector & pivot) {
    //ls_message("Create child %s at:", name.c_str()); pivot.dump();
    Ptr<Bone> child = new Bone(name, this, pivot);
    children.push_back(child);
    return child;
}

void Bone::linkChild(Ptr<Bone> child) {
    children.push_back(child);
    child->parent = this;
}

void Bone::unlinkChild(Ptr<Bone> child) {
    Children::iterator i = find(children.begin(), children.end(), child);
    if (i != children.end()) {
        swap(*i, children.back());
        children.resize(children.size()-1);
        child->parent = 0;
    }
}

Ptr<Bone> Bone::getRoot() {
    Bone * bone = this;
    while (bone->parent) {
        bone = bone->parent;
    }
    
    return bone;
}

void Bone::setTransform(const Transform & xform) {
    local_xform = xform;
    
    if (parent) {
        effective_xform = Transform(Quaternion(1,0,0,0), pivot-parent->pivot) * xform;
    } else {
        effective_xform = xform;
    }
}

void Bone::draw(JRenderer & r) {
    r.pushMatrix();
    r.multMatrix(effective_xform.toMatrix());
    if (object) object->draw(r);
    for(int i=0; i<children.size(); ++i)
        children[i]->draw(r);
    r.popMatrix();
}

Vector Bone::transformPoint(Vector point) {
    point -= pivot;
    Bone * bone = this;
    while(bone->parent) {
        point = bone->effective_xform(point);
        bone = bone->parent;
    }
    point = bone->effective_xform(point);
    return point;
}

Skeleton::~Skeleton() {
    // nothing special to do
}

void Skeleton::load(Ptr<IGame> game, const std::string & filename) throw(invalid_argument) {
    Ptr<IModelMan> modelman = game->getModelMan();
    Ptr<Model> model = 0;

    string dir;
    string::size_type n = filename.rfind('/');
    if ( n == string::npos ) dir = "./";
    else dir = filename.substr(0, n+1);
    
    ifstream in(filename.c_str());
    if (!in)
        throw invalid_argument("Could not open skeleton spec file "+filename);
    
    while(in && !in.eof()) {
        char linebuf[1024];
        in.getline(linebuf, 1024);
        istringstream line(linebuf);
        
        string command;
        line >> command;
        
        if (command == "model") {
            string modelfile;
            line >> modelfile;
            model = modelman->query(dir+"/"+modelfile);
        } else if (command == "bounding_radius") {
            line >> bounding_radius;
        } else if (command == "root") {
            if (!model) throw invalid_argument("Must specify model before root");
            
            string rootname;
            float x,y,z;
            line >> rootname >> x >> y >> z;
            root_bone = new Bone(rootname, Vector(x,y,z));
            root_bone->setObject(model->getObject(rootname));
            bones_by_name[rootname] = root_bone;
            
            if (!root_bone->getObject()) throw invalid_argument("Object not found: "+rootname);
        } else if (command == "childof") {
            if (!root_bone) throw invalid_argument("Must specify root before childof");
            
            string parentname, childname;
            float x,y,z;
            line >> parentname >> childname >> x >> y >> z;
            Ptr<Bone> parent = getBone(parentname);
            if (!parent) throw invalid_argument("Parent "+parentname+" not found");
            Ptr<Bone> bone = parent->createChild(childname, Vector(x,y,z));
            bone->setObject(model->getObject(childname));
            bones_by_name[childname] = bone;
            
            if (!bone->getObject()) throw invalid_argument("Object not found: "+childname);
        } else if (command == "alias") {
            string name, alias;
            line >> name >> alias;
            Ptr<Bone> bone = getBone(name);
            if (!bone) throw invalid_argument("Bone not found in alias: "+name+" -> "+alias);
            bones_by_name[alias] = bone;
        } else if (command == "pointof") {
            string parentname, pointname;
            float x,y,z;
            line >> parentname >> pointname >> x >> y >> z;
            Ptr<Bone> parent = getBone(parentname);
            if (!parent) throw invalid_argument("Parent "+parentname+" not found");
            Point point = make_pair(Vector(x,y,z), parent);
            points_by_name[pointname] = point;
        }
    }
    
    if (!root_bone) throw invalid_argument("Must specify root bone");
}

void Skeleton::setBoneTransform(const std::string & name, const Transform & xform) {
    Ptr<Bone> bone = getBone(name);
    if (!bone) return;
    bone->setTransform(xform);
}

const Transform Skeleton::getBoneTransform(const std::string & name) {
    Ptr<Bone> bone = getBone(name);
    if (!bone) return Transform::identity();
    return bone->getTransform();
}

const Transform Skeleton::getEffectiveBoneTransform(const std::string & name) {
    Ptr<Bone> bone = getBone(name);
    if (!bone) return Transform::identity();
    return bone->getEffectiveTransform();
}


void Skeleton::setRootBoneTransform(const Transform & xform) {
    if (!root_bone) return;
    root_bone->setTransform(xform);
}

Ptr<Bone> Skeleton::getBone(const string & name) {
    BonesByName::iterator i = bones_by_name.find(name);
    if (i == bones_by_name.end()) {
        return 0;
    }
    return i->second;
}

Vector Skeleton::getPoint(const string & name) {
    PointsByName::iterator i = points_by_name.find(name);
    if (i == points_by_name.end()) {
        return Vector(0,0,0);
    }
    return i->second.second->transformPoint(i->second.first);
}

Vector Skeleton::getUntransformedPoint(const string & name) {
    PointsByName::iterator i = points_by_name.find(name);
    if (i == points_by_name.end()) {
        return Vector(0,0,0);
    }
    return i->second.first;
}



void Skeleton::draw(JRenderer & r) {
    float frustum[6][4];
    float dist = 0.0;
    Vector p = root_bone->getTransform().vec();

    // FIXME: It should really _not_ be necessary to get the frustum from the game's camera.
    //        It _should_ be possible to query it directly from the renderer.
    //        Then, we could drop the pointer to the camera!
    camera->getFrustumPlanes(frustum);
    for(int plane=0; plane<6; plane++) {
        float d = 0;
        for(int i=0; i<3; i++) d += frustum[plane][i]*p[i];
        d += frustum[plane][3];
        if (d < -bounding_radius) return; // Out of frustum -> cull!
        if (plane == PLANE_MINUS_Z) dist = d;
    }

    r.setCullMode(JR_CULLMODE_CULL_NEGATIVE);
    root_bone->draw(r);
    r.setCullMode(JR_CULLMODE_NO_CULLING);
}
