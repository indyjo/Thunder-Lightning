#ifndef SOUND_H
#define SOUND_H

#include <string>

class Sound : virtual public Object
{
public:
    virtual float getLengthInSecs() = 0;
};

class SoundSource : virtual public Object
{
public:
    virtual void setPosition(const Vector &) = 0;
    virtual void setVelocity(const Vector &) = 0;
    virtual void setLooping(bool) = 0;
    virtual void setPitch(float) = 0;
    virtual void setGain(float) = 0;
    virtual void setReferenceDistance(float) = 0;
    virtual void setMinGain(float) = 0;
    virtual void setMaxGain(float) = 0;

    virtual void play(Ptr<Sound> snd) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    virtual void rewind() = 0;
    
    virtual Vector getPosition() = 0;
    virtual Vector getVelocity() = 0;
    virtual bool   isLooping() = 0;
    virtual float  getGain() = 0;
    virtual float  getReferenceDistance() = 0;
    virtual float  getMinGain() = 0;
    virtual float  getMaxGain() = 0;
    
    virtual Ptr<Sound> getSound() = 0;
    
    virtual bool isPlaying() = 0;
    virtual bool isPaused() = 0;
    virtual bool isAudible() = 0;
    
    virtual float getCurrentOffsetInSecs() = 0;
    virtual float getEffectiveGain() = 0;
};

class SoundMan : virtual public Object
{
public:
    virtual Ptr<Sound> querySound(const std::string & name) = 0;
    virtual Ptr<SoundSource> requestSource() = 0;
    
    virtual void update(float delta_t) = 0;
    
    virtual void manage(Ptr<SoundSource>) = 0;
    
    virtual void setListenerPosition( const Vector & pos ) = 0;
    virtual void setListenerVelocity( const Vector & vel ) = 0;
    virtual void setListenerOrientation( const Vector & up, const Vector & front ) = 0;
    
    virtual Vector getListenerPosition() = 0;
    virtual Vector getListenerVelocity() = 0;
    virtual void getListenerOrientation( Vector & up, Vector & front) = 0;
    
    virtual void flush() = 0;
    virtual void shutdown() = 0;
};

#endif
