#ifndef _JOGI_SPRITE_H
#define _JOGI_SPRITE_H

#include <fstream>
#include "types.h"

class JSprite {
public:
    jsprite_t sprite;

    JSprite();
    ~JSprite();
    
    jError load(std::istream &in);
    jError save(std::ostream &out);
    jError load(const char *filename);
    jError save(const char *filename);
    
protected:
    jError loadSPR(std::istream &in);
    jError loadPNG(std::istream &in);
    void unload();
};
    
#endif

