#ifndef ICONFIG_H
#define ICONFIG_H

#include <object.h>
#include <modules/math/Vector.h>

struct IConfig : virtual public Object
{
    virtual void feedArguments(int argc, const char **const argv)=0;
    
    virtual const char *query(const char *key, const char *def =0)=0;
    virtual float  queryFloat(const char *key, float def=0.0f)=0;
    virtual int    queryInt(const char *key, int def=0)=0;
    virtual bool   queryBool(const char *key, bool def=false)=0;
    virtual Vector queryVector(const char *key, Vector def=Vector(0,0,0))=0;

    virtual void set(const char *key, const char *value)=0;
};

#endif
