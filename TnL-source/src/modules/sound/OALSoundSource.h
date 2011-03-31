#ifndef SOUND_OALSOUNDSOURCE_H
#define SOUND_OALSOUNDSOURCE_H

#include <AL/al.h>
#include "OALSound.h"
#include "SoundSource.h"

class OALSoundSource : public SoundSource
{
    ALuint source;
    Ptr<OALSound> sound;

public:
    const static Type type;
    
    OALSoundSource();
    virtual ~OALSoundSource();
    
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
    
    // this is dependent on OpenAL version and handled different in child classes
    virtual float       getCurrentOffsetInSecs()=0;
    
    void getEffectiveGain(const Vector & listener);
};

class OAL10SoundSource : public OALSoundSource {
    float current_offset;
    
public:
    const static Type type;
    
    OAL10SoundSource();
    ~OAL10SoundSource();
    
    void update(float delta_t);
    
    float getCurrentOffsetInSecs();
};

class OAL11SoundSource : public OALSoundSource {
public:
    const static Type type;
    
    OAL11SoundSource();
    ~OAL11SoundSource();
    
    float getCurrentOffsetInSecs();
    void  setCurrentOffsetInSecs(float secs);
};

#endif

