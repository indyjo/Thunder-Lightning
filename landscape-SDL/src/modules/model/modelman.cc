#include <fstream>
#include "modelman.h"

using namespace std;

ModelMan::ModelMan(Ptr<TextureManager> texman) : texman(texman) { }

Ptr<Model> ModelMan::query(const string & name) {
    // Check if already loaded and return
    ModelMap::iterator i = models.find(name);
    if (i != models.end()) return i->second;
    
    // Model is not in the map -> we have to load it
    // First extract the directory out 
    string::size_type n = name.rfind('/');
    string dir;
    if ( n == string::npos ) dir = "./";
    else dir = name.substr(0, n+1);
    
    // Open the model file and check if it is valid
    ifstream in(name.c_str());
    if (!in) {
        ls_warning("ModelMan::query(): Couldn't load model %s!\n",
                name.c_str());
        return 0; // Baaaad.
    }
    
    // Everything seems to be OK. Now load the model
    Ptr<Model> model = new Model(*texman, in, dir.c_str());
    models[name] = model;
    return model;
}

void ModelMan::flush() {
    models.clear();
}
