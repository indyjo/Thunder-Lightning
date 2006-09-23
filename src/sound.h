#ifndef SOUND_H
#define SOUND_H

#include <map>
#include <vector>
#include <al.h>
#include <alc.h>
#include <Weak.h>
#include <modules/math/Vector.h>

struct IConfig;
class SoundMan;

class Sound : virtual public Object
{
    ALuint buffer;
public:
    Sound( const std::string & filename );
    ~Sound();
    
    float getLengthInSecs();
    
protected:
    friend class SoundSource;
    friend class SoundMan;
    inline ALuint getResource() { return buffer; }
};

class SoundSource : public Object, public Weak
{
    ALuint source;
    Ptr<SoundMan> soundman;
    Ptr<Sound> sound;
    
    /// Caches state while inaudible
    ALenum state;
    float cached_offset;
    bool audible;
    
    SoundSource(Ptr<SoundMan> soundman);
public:
    virtual ~SoundSource();

    void setPosition(const Vector &);
    void setVelocity(const Vector &);
    void setLooping(bool);
    void setPitch(float);
    void setGain(float);
    void setReferenceDistance(float);

    void play(Ptr<Sound> snd);
    void pause();
    void resume();
    void stop();
    void rewind();
    
    Vector getPosition();
    Vector getVelocity();
    bool   isLooping();
    float  getGain();
    float  getReferenceDistance();
    
    inline const Ptr<Sound> & getSound() {return sound; }
    
    bool isPlaying();
    bool isPaused();
    inline bool isAudible() { return audible; }
    
    float getCurrentOffsetInSecs();
    float getEffectiveGain();
    
private:
    friend class SoundMan;
        
    inline ALuint getResource() { return source; }
    void setAudible(bool);
    void update(float delta_t);
};

class SoundMan : public Object
{
    /// OpenAL version number
    /// Some functions are not supported on OpenAL1.0
    int openal_major, openal_minor;
    std::map<std::string, Ptr<Sound> > sounds;
    
    std::vector<Ptr<SoundSource> > managed_sources;
    std::vector<WeakPtr<SoundSource> > all_sources;
    
    std::string sound_dir;
    
    ALCdevice * device;
    void * context;
    int play_channels;
    int playing_sources;
    
    float minimum_gain, hysteresis;
    
	friend class SoundSource;

public:

    SoundMan(Ptr<IConfig>);
    ~SoundMan();

    Ptr<Sound> querySound(const std::string & name);
    Ptr<SoundSource> requestSource();
    
    void update(float delta_t);
    
    void manage(Ptr<SoundSource>);
    
    void setListenerPosition( const Vector & pos );
    void setListenerVelocity( const Vector & vel );
    void setListenerOrientation( const Vector & up, const Vector & front );
    
    Vector getListenerPosition();
    Vector getListenerVelocity();
    void getListenerOrientation( Vector & up, Vector & front);
    
    void flush();
    void shutdown();
protected:
    
};

#endif
