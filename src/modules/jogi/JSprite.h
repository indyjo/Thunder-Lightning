#ifndef _JOGI_SPRITE_H
#define _JOGI_SPRITE_H

#include <fstream>

class JSprite {
public:
    jsprite_t sprite;

    JSprite();
    ~JSprite();
    
    jError load(std::istream &in);
    jError save(std::ostream &out);
    
protected:
    jError loadSPR(std::istream &in);
    jError loadPNG(std::istream &in);
    void unload();
};
    
#endif

