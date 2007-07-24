#ifndef SOUND_PROXYSOUNDSOURCE_H
#define SOUND_PROXYSOUNDSOURCE_H

#include "SoundSource.h"


class ProxySoundSource : public SoundSource
{
    Ptr<SoundSource> target;
public:
    const static Type type;

    ProxySoundSource(Ptr<SoundSource> target);
    virtual ~ProxySoundSource();
    
    Ptr<SoundSource>    getTarget();
    void                setTarget(Ptr<SoundSource>);

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

