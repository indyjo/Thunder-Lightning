#include <fstream>
#include "modelman.h"

using namespace std;

ModelMan::ModelMan(Ptr<TextureManager> texman) : texman(texman) { }

Ptr<Model> ModelMan::query(const string & name) {
    // Check if already loaded and return
    ModelMap::iterator i = models.find(name);
    if (i != models.end()) return i->second;
    
    // Everything seems to be OK. Now load the model
    Ptr<Model> model = new Model(*texman, name);
    models[name] = model;
    return model;
}

void ModelMan::flush() {
    models.clear();
}
