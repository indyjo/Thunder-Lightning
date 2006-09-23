#ifndef SKELETON_PROVIDER_H
#define SKELETON_PROVIDER_H

#include <interfaces/IPositionProvider.h>
#include <landscape.h>
#include "Skeleton.h"

struct SkeletonProvider : public IPositionProvider {
    Ptr<Skeleton> skeleton;
    const char * pos, *front1, *front2, *up1, *up2;
    
    SkeletonProvider(Ptr<Skeleton> skel, const char *p, const char *f1, const char *f2,
        const char* u1=0, const char *u2=0);
    
    virtual Vector getLocation();
    Matrix3 orient();
    virtual Vector getFrontVector();
    virtual Vector getRightVector();
    virtual Vector getUpVector();
    virtual void getOrientation(Vector *up, Vector *right, Vector *front);
};


#endif
