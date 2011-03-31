#ifndef IMODELMAN_H
#define IMODELMAN_H

#include <string>
#include <object.h>

class Model;

struct IModelMan : virtual public Object
{
    virtual Ptr<Model> query(const std::string & name)=0;
    virtual void flush()=0;
};

#endif
