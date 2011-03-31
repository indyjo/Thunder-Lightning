#ifndef SOUND_VIRTUALSOUNDSOURCE_H
#define SOUND_VIRTUALSOUNDSOURCE_H

#include "SoundSource.h"


class VirtualSoundSource : public SoundSource
{
    Ptr<Sound> sound;
    Vector position, velocity;
    bool is_looping, is_playing, is_paused;
    float pitch, gain, refdist, mingain, maxgain;
    float offset;
public:
    const static Type type;

    VirtualSoundSource();
    virtual ~VirtualSoundSource();

    void cloneFrom(Ptr<SoundSource> other);
    
    virtual Vector  getPosition();
    virtual void    setPosition(const Vector &);
    
    virtual Vector  getVelocity();
    virtual void    setVelocity(const Vector &);
    
    virtual bool    isLooping();
    virtual void    setLooping(bool);
    
    virtual float   getPitch();
    virtual void    setPitch(float);
    
    virtual float   getGain();
    virtual void    setGain(float);
    
    virtual float   getReferenceDistance();
    virtual void    setReferenceDistance(float);
    
    virtual float   getMinGain();
    virtual void    setMinGain(float);

    virtual float   getMaxGain();
    virtual void    setMaxGain(float);

    virtual void    play(Ptr<Sound> snd);
    virtual void    pause();
    virtual void    resume();
    virtual void    stop();
    virtual void    rewind();
    
    virtual bool    isPlaying();
    virtual bool    isPaused();
    
    virtual Ptr<Sound>  getCurrentSound();
    virtual float       getCurrentOffsetInSecs();
};


#endif

