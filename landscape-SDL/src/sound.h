#ifndef SOUND_H
#define SOUND_H

#include <map>
#include <deque>
#include <AL/al.h>
#include <AL/alc.h>
#include <object.h>
#include <modules/math/Vector.h>

class Sound : virtual public Object
{
    ALuint buffer;
public:
    Sound( const std::string & filename );
    ~Sound();
    
protected:
    friend class SoundSource;
    friend class SoundMan;
    inline ALuint getResource() { return buffer; }
};

class SoundSource : virtual public Object
{
    ALuint source;
    Ptr<SoundMan> soundman;
    
    SoundSource(Ptr<SoundMan> soundman);
public:
    virtual ~SoundSource();
    
    void setPosition(const Vector &);
    void setVelocity(const Vector &);
    void setLooping(bool);
    void setPitch(float);
    void setGain(float);
    
    void play(Ptr<Sound> snd);
    void pause();
    void stop();
    void rewind();
    
    bool isPlaying();
    
protected:
    friend class SoundMan;
        
    inline ALuint getResource() { return source; }
};

class SoundMan : virtual public Object
{
    std::map<std::string, Ptr<Sound> > sounds;
    std::vector<Ptr<SoundSource> > playing_sources;
    
    std::string sound_dir;
    
    ALCdevice * device;
    void * context;
    int play_channels;
    
public:
    SoundMan(const std::string & sound_dir);
    ~SoundMan();

    Ptr<Sound> querySound(const std::string & name);
    Ptr<SoundSource> requestSource();
    
    bool requestPlayChannel(Ptr<SoundSource> source);
    void update();
    
    void setListenerPosition( const Vector & pos );
    void setListenerVelocity( const Vector & vel );
    void setListenerOrientation( const Vector & up, const Vector & front );    
    
    void flush();
protected:
    
};

#endif
