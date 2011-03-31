#include <interfaces/IConfig.h>

#include "OALbase.h"
#include "OALSoundMan.h"

OALSoundMan::OALSoundMan(Ptr<IConfig> config)
:	sound_dir(config->query("SoundMan_sound_dir"))
{
    ls_message("Initializing OALSoundMan... ");

    device = alcOpenDevice( NULL );
    check();
    if (device == NULL ) {
        throw std::runtime_error("SoundMan: Failed to initialize Sound subsystem.");
    } else {
        const ALCbyte * device_specifier =
            alcGetString(device, ALC_DEVICE_SPECIFIER);
        check(device);
        ls_message("  Using device \"%s\"\n", device_specifier);
    }
    
    context = alcCreateContext( device, NULL );
    check();
    if (context == NULL ) {
        throw std::runtime_error("Soundman: Couldn't open audio context.");
    }
    alcMakeContextCurrent( (ALCcontext*) context );
    check(device);
    
    alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, &openal_major);
    check(device);
    alcGetIntegerv(device, ALC_MINOR_VERSION, 1, &openal_minor);
    check(device);
    ls_message("  OpenAL version: %d %d\n", openal_major, openal_minor);
    
    if (openal_major < 1 || openal_major==1 && openal_minor <1) {
        ls_warning("  Warning: OpenAL Version 1.1 recommended!\n");
        ls_warning("           Some functions will be disabled.\n");
    }

    //openal_major = 1; openal_minor = 0;
    
    ls_message("  Device specifier of default OpenAL device: \"%s\"\n",
        alcGetString(device, ALC_DEFAULT_DEVICE_SPECIFIER));
    check(device);
    
    ls_message("  Supported OpenAL extensions: [%s]\n",
        alcGetString(device, ALC_EXTENSIONS));
    check(device);
    
    alDopplerVelocity(config->queryFloat("SoundMan_doppler_velocity",10000.0));
    alDopplerFactor(config->queryFloat("SoundMan_doppler_factor", 1));
    alDistanceModel(AL_INVERSE_DISTANCE);
    play_channels = config->queryInt("SoundMan_channels", 32);
    minimum_gain = config->queryFloat("SoundMan_minimum_gain", 0.001f);
    hysteresis = config->queryFloat("SoundMan_hysteresis", 0.0001f);
    
    alutInitWithoutContext(NULL, NULL);
    ls_message("done.\n");
}

SoundMan::~SoundMan() {
    alutExit();
    alcDestroyContext( (ALCcontext*) context );
    alcCloseDevice( device );
    ls_message("SoundMan shutdown complete.\n");
}

Ptr<Sound> SoundMan::querySound(const string & name) {
    if (sounds.find(name) == sounds.end())
        sounds[name] = new Sound(sound_dir + "/" + name);
    return sounds[name];
}

Ptr<SoundSource> SoundMan::requestSource() {
    Ptr<SoundSource> s = new SoundSource(this);
    all_sources.push_back(ptr(s));
    
    if (   openal_major < 1 
        || openal_major == 1 && openal_minor < 1)
    {
        // On OpenAL 1.0 and below, source offset is not supported.
        // Thus, we can only make sources inaudible, never again audible
        if (playing_sources < play_channels) {
            s->setAudible(true);
            playing_sources++;
        } else {
            s->setAudible(false);
        }
    } else {
        // On OpenAL 1.1 and later, we use a sophisticated source management
        if (playing_sources < play_channels
            && s->getEffectiveGain() > minimum_gain)
        {
            s->setAudible(true);
            playing_sources++;
        } else {
            s->setAudible(false);
        }
    }
    return s;
}

bool invalid(WeakPtr<SoundSource> s) { return !s.valid(); }
bool playing(WeakPtr<SoundSource> s) {
    Ptr<SoundSource> p = s.lock();
    return p && p->isAudible() && p->isPlaying();
}
bool not_playing(Ptr<SoundSource> p) { return !p->isPlaying(); }

void SoundMan::update(float delta_t) {
    typedef vector<WeakPtr<SoundSource> >::iterator WSI;
    typedef vector<Ptr<SoundSource> >::iterator PSI;

    // remove all managed sources no longer playing
    managed_sources.resize(
        remove_if(
            managed_sources.begin(),
            managed_sources.end(),
            not_playing)
        - managed_sources.begin());
    
    // remove all deleted sources from the list of all sources
    all_sources.resize(
        remove_if(
            all_sources.begin(),
            all_sources.end(),
            invalid)
        - all_sources.begin());
        
    // now all_sources contains only valid sources
    
    // count the number of currently audibly playing sources
    playing_sources = std::count_if(
        all_sources.begin(),
        all_sources.end(),
        playing);

    // With OpenAL implementations >= 1.1 we enter source managing now.
    // For 1.0 and below, we keep sources audible all the time.
    if (openal_major  < 1 ||
        openal_major == 1 && openal_minor < 1)
    {
        return;
    }
        
    // make very silent audible sources inaudible
    // make not-so-silent inaudible sources audible, provided there
    //      are enough channels left
    for(WSI i=all_sources.begin(); i!= all_sources.end(); ++i) {
        SoundSource *source = ptr(*i);
        float gain = source->getEffectiveGain();
        if (source->isAudible() && gain < minimum_gain) {
            source->setAudible(false);
            if (source->isPlaying()) --playing_sources;
        } else if(!source->isAudible()
                  && source->isPlaying()
                  && gain > minimum_gain+hysteresis
                  && playing_sources < play_channels )
        {
            source->setAudible(true);
            ++playing_sources;
        }
    }
    //ls_message("%d playing sources.\n", playing_sources);
    
    for(WSI i=all_sources.begin(); i!= all_sources.end(); ++i) {
        SoundSource *source = ptr(*i);
        source->update(delta_t);
    }
}            

void SoundMan::manage(Ptr<SoundSource> src) {
	managed_sources.push_back(src);
}

void SoundMan::setListenerPosition( const Vector & pos ) {
    alListener3f( AL_POSITION, pos[0], pos[1], pos[2]);
}

void SoundMan::setListenerVelocity( const Vector & vel ) {
    alListener3f( AL_VELOCITY, vel[0], vel[1], vel[2]);
}

void SoundMan::setListenerOrientation(
        const Vector & up, const Vector & front ) {
    float upfront[6];
    
    for (int i=0; i<3; i++) upfront[i] = -front[i];
    for (int i=3; i<6; i++) upfront[i] = up[i-3];
    
    alListenerfv( AL_ORIENTATION, upfront);
}

Vector SoundMan::getListenerPosition() {
    float vec[3];
    alGetListenerfv(AL_POSITION, vec);
    return Vector(vec[0],vec[1],vec[2]);
}

Vector SoundMan::getListenerVelocity() {
    float vec[3];
    alGetListenerfv(AL_VELOCITY, vec);
    return Vector(vec[0],vec[1],vec[2]);
}

void SoundMan::getListenerOrientation(Vector & up, Vector & front) {
    float vec[6];
    alGetListenerfv(AL_ORIENTATION, vec);
    up    = Vector(vec[0],vec[1],vec[2]);
    front = Vector(vec[3],vec[4],vec[5]);
}

void SoundMan::flush() {
    sounds.clear();
}

void SoundMan::shutdown() {
	typedef vector<Ptr<SoundSource> > Sources;
	typedef Sources::iterator SIter;
	
	flush();
}	
