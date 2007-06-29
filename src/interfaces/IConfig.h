#ifndef ICONFIG_H
#define ICONFIG_H

#include <string>
#include <object.h>
#include <modules/math/Vector.h>

struct IConfig : virtual public Object
{
    virtual void feedArguments(int argc, const char **argv)=0;
    
    virtual const char *query(const std::string& key, const char* def =0)=0;
    virtual float  queryFloat(const std::string& key, float def=0.0f)=0;
    virtual int    queryInt(const std::string& key, int def=0)=0;
    virtual bool   queryBool(const std::string& key, bool def=false)=0;
    virtual Vector queryVector(const std::string& key, Vector def=Vector(0,0,0))=0;

    virtual void set(const std::string& key, const std::string& value)=0;
};

#endif
