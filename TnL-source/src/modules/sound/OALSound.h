#ifndef SOUND_OALSOUND_H
#define SOUND_OALSOUND_H

#include <string>
#include <AL/al.h>
#include "Sound.h"

Type OALSoundType;

class OALSound : public Sound
{
    ALuint buffer;
public:
    const static Type type;
    
    OALSound( const std::string & filename );
    ~OALSound();
    
    float getLengthInSecs();
    inline ALuint getResource() { return buffer; }
};

#endif

