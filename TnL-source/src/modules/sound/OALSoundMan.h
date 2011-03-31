#ifndef SOUND_OALSOUNDMAN_H
#define SOUND_OALSOUNDMAN_H

#include "SoundMan.h"
#include "ProxySoundSource.h"

struct IConfig;

class OALSoundMan : public SoundMan
{
    std::map<std::string, Ptr<Sound> > sounds;
    std::vector<Ptr<SoundSource> > managed_sources;
    
    typedef std::vector<WeakPtr<ProxySoundSource> > WeakProxyList;
    WeakProxyList physical_sources, virtual_sources;
    
    std::string sound_dir;
    ALCdevice * device;
    void * context;
    
    int max_physical_sources;
    float minimum_gain, hysteresis;
    
public:
    const static Type type;

    OALSoundMan(Ptr<IConfig>);
    ~OALSoundMan();

    virtual Ptr<Sound> querySound(const std::string & name);
    
    virtual void update(float delta_t);
    
    virtual void manage(Ptr<SoundSource>);
    virtual void removeFromManaged(Ptr<SoundSource);
    
    virtual void setListenerPosition( const Vector & pos );
    virtual void setListenerVelocity( const Vector & vel );
    virtual void setListenerOrientation( const Vector & up, const Vector & front );
    
    virtual Vector getListenerPosition();
    virtual Vector getListenerVelocity();
    virtual void getListenerOrientation( Vector & up, Vector & front);
    
    void shutdown();
protected:
    
};

class OAL10SoundMan : public OALSoundMan
{
public:
    const static Type type;
    
    OAL10SoundMan(Ptr<IConfig>);
    
    Ptr<SoundSource> requestSource();
    virtual void update(float delta_t);
};

class OAL11SoundMan : public OALSoundMan
{
public:
    const static Type type;
    Ptr<SoundSource> requestSource();
};

#endif

