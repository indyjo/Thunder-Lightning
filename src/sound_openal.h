#ifndef SOUND_OPENAL_H
#define SOUND_OPENAL_H

#include <map>
#include <vector>
#include <al.h>
#include <alc.h>
#include <modules/math/Vector.h>
#include <Weak.h>

#include "sound.h"

struct IConfig;
class ALSoundMan;

class ALSound : public Sound
{
    ALuint buffer;
public:
    ALSound( const std::string & filename );
    ~ALSound();
    
    virtual float getLengthInSecs();
    
protected:
    friend class ALSoundSource;
    friend class ALSoundMan;
    inline ALuint getResource() { return buffer; }
};

class ALSoundSource : public SoundSource, virtual public Weak
{
    ALuint source;
    Ptr<ALSoundMan> soundman;
    Ptr<ALSound> sound;
    
    /// Caches state while inaudible
    ALenum state;
    float cached_offset;
    bool audible;
    
    ALSoundSource(Ptr<ALSoundMan> soundman);
public:
    virtual ~ALSoundSource();

    virtual void setPosition(const Vector &);
    virtual void setVelocity(const Vector &);
    virtual void setLooping(bool);
    virtual void setPitch(float);
    virtual void setGain(float);
    virtual void setReferenceDistance(float);
    virtual void setMinGain(float);
    virtual void setMaxGain(float);

    virtual void play(Ptr<Sound> snd);
    virtual void pause();
    virtual void resume();
    virtual void stop();
    virtual void rewind();
    
    virtual Vector getPosition();
    virtual Vector getVelocity();
    virtual bool   isLooping();
    virtual float  getGain();
    virtual float  getReferenceDistance();
    virtual float  getMinGain();
    virtual float  getMaxGain();
    
    inline Ptr<Sound> getSound() {return sound; }
    
    virtual bool isPlaying();
    virtual bool isPaused();
    inline bool isAudible() { return audible; }
    
    virtual float getCurrentOffsetInSecs();
    virtual float getEffectiveGain();
    
private:
    friend class ALSoundMan;
        
    inline ALuint getResource() { return source; }
    void setAudible(bool);
    void update(float delta_t);
};

class ALSoundMan : public SoundMan
{
    /// OpenAL version number
    /// Some functions are not supported on OpenAL1.0
    int openal_major, openal_minor;
    std::map<std::string, Ptr<ALSound> > sounds;
    
    std::vector<Ptr<ALSoundSource> > managed_sources;
    std::vector<WeakPtr<ALSoundSource> > all_sources;
    
    std::string sound_dir;
    
    ALCdevice * device;
    void * context;
    int play_channels;
    int playing_sources;
    
    float minimum_gain, hysteresis;
    
	friend class ALSoundSource;

public:

    ALSoundMan(Ptr<IConfig>);
    ~ALSoundMan();

    virtual Ptr<Sound> querySound(const std::string & name);
    virtual Ptr<SoundSource> requestSource();
    
    virtual void update(float delta_t);
    
    virtual void manage(Ptr<SoundSource>);
    
    virtual void setListenerPosition( const Vector & pos );
    virtual void setListenerVelocity( const Vector & vel );
    virtual void setListenerOrientation( const Vector & up, const Vector & front );
    
    virtual Vector getListenerPosition();
    virtual Vector getListenerVelocity();
    virtual void getListenerOrientation( Vector & up, Vector & front);
    
    virtual void flush();
    virtual void shutdown();
};

#endif
