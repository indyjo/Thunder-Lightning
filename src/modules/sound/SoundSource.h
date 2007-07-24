#ifndef SOUND_SOUNDSOURCE_H
#define SOUND_SOUNDSOURCE_H

#include <modules/math/Vector.h>
#include <TypedObject.h>
#include <object.h>
#include "Sound.h"

class SoundSource : public Object, public TypedObject, public Weak
{
public:
    const static Type type;

    virtual ~SoundSource()=0;

    virtual Vector  getPosition()=0;
    virtual void    setPosition(const Vector &)=0;
    
    virtual Vector  getVelocity()=0;
    virtual void    setVelocity(const Vector &)=0;
    
    virtual bool    isLooping()=0;
    virtual void    setLooping(bool)=0;
    
    virtual float   getPitch()=0;
    virtual void    setPitch(float)=0;
    
    virtual float   getGain()=0;
    virtual void    setGain(float)=0;
    
    virtual float   getReferenceDistance()=0;
    virtual void    setReferenceDistance(float)=0;
    
    virtual float   getMinGain()=0;
    virtual void    setMinGain(float)=0;

    virtual float   getMaxGain()=0;
    virtual void    setMaxGain(float)=0;

    virtual void    play(Ptr<Sound> snd)=0;
    virtual void    pause()=0;
    virtual void    resume()=0;
    virtual void    stop()=0;
    virtual void    rewind()=0;
    
    virtual bool    isPlaying()=0;
    virtual bool    isPaused()=0;
    
    virtual Ptr<Sound>  getCurrentSound()=0;
    virtual float       getCurrentOffsetInSecs()=0;
};

#endif

