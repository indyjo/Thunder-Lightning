#ifndef MODELMAN_H
#define MODELMAN_H

#include <map>
#include <string>

#include <landscape.h>
#include "model.h"

class ModelMan : virtual public IModelMan {
    typedef std::map<std::string, Ptr<Model> > ModelMap;
    ModelMap models;
    Ptr<TextureManager> texman;
public:
    ModelMan( Ptr<TextureManager> texman );
    virtual Ptr<Model> query(const std::string & name);
    virtual void flush();
};

#endif
