#ifndef SOUND_SOUND_H
#define SOUND_SOUND_H

#include <object.h>
#include <TypedObject.h>

class Sound : public Object, public TypedObject
{
public:
    const static Type type;
    
    virtual ~Sound()=0;
    
    virtual float getLengthInSecs()=0;
};

#endif

