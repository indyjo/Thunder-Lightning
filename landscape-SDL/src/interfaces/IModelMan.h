#ifndef IMODELMAN_H
#define IMODELMAN_H

#include <string>
#include <object.h>

class Model;

class IModelMan : virtual public Object
{
public:
    virtual Ptr<Model> query(const std::string & name)=0;
    virtual void flush()=0;
};

#endif
