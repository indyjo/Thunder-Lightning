#ifndef SKELETON_H
#define SKELETON_H

#include <map>
#include <string>
#include <stdexcept>
#include <vector>
#include <interfaces/IGame.h>
#include <modules/math/Transform.h>
#include "model.h"

struct JRenderer;

class Bone : public Object {
    typedef std::vector<Ptr<Bone> > Children;
    
    Bone *parent;
    std::string name;
    Vector pivot;
    Children children;
    Transform local_xform;
    Transform effective_xform;
    Ptr<Model::Object> object;
    
private:
    Bone(const std::string & name, Bone *parent, const Vector & pivot);
   
public:
    Bone(const std::string & name, const Vector & pivot);
    
    /// Creates a child bone which is automatically linked to this bone.
    /// returns the new bone
    Ptr<Bone> createChild(const std::string & name, const Vector & pivot);
    /// Links an unlinked (root) bone as a child to this bone
    void linkChild(Ptr<Bone>);
    /// Unlinks a child bone from this bone. The former child bone will become root bone
    void unlinkChild(Ptr<Bone>);
    
    inline const std::string & getName() { return name; }
    inline Ptr<Bone> getParent() { return parent; }
    Ptr<Bone> getRoot();
    inline void setObject(Ptr<Model::Object> obj) { object = obj; }
    inline Ptr<Model::Object> getObject() { return object; }
    
    /// Set the local transform.
    /// The local transform is local to the bone's pivot point.
    /// setTransform also calculates the effective transform, which also incorporates
    /// the translation into the pivot system of the parent bone
    void setTransform(const Transform & xform);
    inline const Transform & getTransform() { return local_xform; }
    inline const Transform & getEffectiveTransform() { return effective_xform; }

    /// Transforms a point given in world coordinates.
    /// All transformations up to the root bone will be applied.
    /// Example: Say we are modeling a human. In rest position, one finger tip
    /// will be at (x,y,z) in world coordinates. Assume this bone models the last segment
    /// of the finger. Then transformPoint(Vector(x,y,z)) will give the exact location
    /// of the finger tip with all transformations applied in world coordinates.
    Vector transformPoint(Vector point);
    
    /// Transforms a point given in world coordinates into coordinates local to the current bone's
    /// pivot with all transformations applied.
    Vector localizePoint(Vector point);
    
    void draw(JRenderer & r);
};



class Skeleton : public Object {
    typedef std::pair<Vector, Ptr<Bone> > Point;
    typedef std::map<std::string, Ptr<Bone> > BonesByName;
    typedef std::map<std::string, Point > PointsByName;
    
    BonesByName  bones_by_name;
    PointsByName points_by_name;
    Ptr<Bone>    root_bone;
    
public:
    /// Constructs a skeleton given the name of a skeleton specfile
    inline Skeleton(Ptr<IGame> game, const std::string & filename) throw(std::invalid_argument)
    { load(game, filename); }
    
    /// Sets the local transform of the bone with the given name.
    /// If the name cannot be resolved, does nothing.
    void setBoneTransform(const std::string &, const Transform &);
    
    /// Returns the bone with the given name.
    /// If the name cannot be resolved, returns 0
    Ptr<Bone> getBone(const std::string &);
    
    /// Returns the point with the given name after bone transformations
    Vector getPoint(const std::string &);
    
    void draw(JRenderer & r);
    
private:
    void load(Ptr<IGame> game, const std::string & filename) throw(std::invalid_argument) ;
};


#endif
