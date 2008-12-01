#include <stdexcept>
#include <iostream>
#include <string>
#include <algorithm>

#include <tnl.h>
#include <interfaces/IConfig.h>
#include "sound_openal.h"

//#include <AL/alu.h>
#include <AL/alut.h>

using namespace std;

#ifdef NDEBUG
// The release version causes a warning and the values are ignored
#define CHECKFLOAT(f)                                               \
    if ((f) != (f) || 2*(f) == (f) && (f) != 0)                     \
    {                                                               \
        ls_warning("Invalid float value detected: ignored.\n");       \
        return;                                                     \
    }
#else
// The debug version causes a segfault to enable debugging
#define CHECKFLOAT(f) if ((f) != (f) || 2*(f) == (f) && (f) != 0) { *(char*)0x0 = 'X'; }
#endif

#define CHECKVECTOR(v) CHECKFLOAT(v[0]); CHECKFLOAT(v[1]); CHECKFLOAT(v[2]);

static void alCheck() {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        ls_error("OpenAL Error %d", error);
        const ALchar * errtxt = alGetString(error);
        error = alGetError();
        if (error == AL_NO_ERROR) {
            ls_error(": %s \n", errtxt);
        } else {
            ls_error(". Additionally, an error %d occurred"
                     " while reading the error text.\n", error);
        }
    }
}


ALSound::ALSound( const string & filename ) {
    ls_message("Sound::Sound('%s')\n", filename.c_str());
    alGetError(); // reset error state
    buffer = alutCreateBufferFromFile(filename.c_str());
    ALenum e = alGetError();
    if (e != AL_NO_ERROR) {
        ls_error("Sound error: \n");
        switch(e) {
        //case AL_INVALID_OPERATION: ls_error("INVALID_OPERATION\n"); break;
        case AL_OUT_OF_MEMORY: ls_error("OUT_OF_MEMORY\n"); break;
        default: ls_error("UNKNOWN ERROR\n");
        }
    }
}

ALSound::~ALSound() {
    alDeleteBuffers( 1, &buffer );
}

float ALSound::getLengthInSecs() {
    ALint size, bits, freq;
    alGetBufferi(buffer, AL_SIZE, &size);
    alGetBufferi(buffer, AL_BITS, &bits);
    alGetBufferi(buffer, AL_FREQUENCY, &freq);
    
    return size / (bits/8) / (float) freq;
}

static int nsources=0;
ALSoundSource::ALSoundSource(Ptr<ALSoundMan> soundman)
:   soundman(soundman),
    state(AL_INITIAL),
    audible(true),
    cached_offset(0)
{
    //ls_message("nsources: %d\n",++nsources);
    alGenSources( 1, & source );
    alCheck();
    setReferenceDistance(50);
    //alSourcef(source, AL_MIN_GAIN, 0.0f);
    //ls_message("Created sound source %p\n", this);
}

ALSoundSource::~ALSoundSource() {
    //ls_message("nsources: %d (last source audible: %s)\n",--nsources, audible?"yes":"no");
    if (isPlaying()) stop();
    alDeleteSources(1, & source );
    //ls_message("Deleted sound source %p\n", this);
}

void ALSoundSource::setPosition(const Vector & pos) {
    alSource3f(source, AL_POSITION, pos[0], pos[1], pos[2]);
    CHECKVECTOR(pos);
    if (pos[0]!=pos[0])
        ls_error("ALSound source %p position %f %f %f\n", this, pos[0],pos[1],pos[2]);
}

void ALSoundSource::setVelocity(const Vector & vel) {
    alSource3f(source, AL_VELOCITY, vel[0], vel[1], vel[2]);
    CHECKVECTOR(vel);
    if (vel[0]!=vel[0])
        ls_message("ALSound source %p velocity %f %f %f\n", this, vel[0],vel[1],vel[2]);
}

void ALSoundSource::setLooping(bool loop) {
    alSourcei(source, AL_LOOPING, loop?AL_TRUE:AL_FALSE);
}

void ALSoundSource::setPitch(float pitch) {
    alSourcef(source, AL_PITCH, pitch);
}

void ALSoundSource::setGain(float gain) {
    alSourcef(source, AL_GAIN, gain );
}

void ALSoundSource::setReferenceDistance(float dist) {
    alSourcef(source, AL_REFERENCE_DISTANCE, dist);
}

void ALSoundSource::setMinGain(float gain) {
    alSourcef(source, AL_MIN_GAIN, gain );
}

void ALSoundSource::setMaxGain(float gain) {
    alSourcef(source, AL_MAX_GAIN, gain );
}

void ALSoundSource::play(Ptr<Sound> _sound) {
    Ptr<ALSound> sound = (Ptr<ALSound>) sound;
    this->sound = sound;
    ALuint buffer = sound->getResource();
    alSourceQueueBuffers( source, 1, & buffer );
    if (audible) {
        alSourcePlay(source);
    }
    
    if (!audible) {
        //alSourceRewind(source);
        state = AL_PLAYING;
        cached_offset = 0.0f;
    }
}

void ALSoundSource::pause() {
    if (audible) {
        alSourcePause(source);
    } else {
        if (state == AL_PLAYING) state = AL_PAUSED;
    }
}

void ALSoundSource::resume() {
    if (audible) {
        alSourcePlay(source);
    } else {
        if (state == AL_PAUSED) state = AL_PLAYING;
    }
}

void ALSoundSource::stop() {
    if (audible) {
        alSourceStop(source);
    } else {
        state = AL_INITIAL;
        //alSourceRewind(source);
        cached_offset = 0.0f;
    }
}

void ALSoundSource::rewind() {
    if (audible) {
        alSourceRewind(source);
    } else {
        state = AL_INITIAL;
        cached_offset = 0.0f;
        //alSourceRewind(source);
    }
}

Vector ALSoundSource::getPosition() {
    float vec[3];
    alGetSourcefv(source, AL_POSITION, vec);
    return Vector(vec[0], vec[1], vec[2]);
}

Vector ALSoundSource::getVelocity() {
    float vec[3];
    alGetSourcefv(source, AL_VELOCITY, vec);
    return Vector(vec[0], vec[1], vec[2]);
}

bool ALSoundSource::isLooping() {
    ALint b;
    alGetSourcei(source, AL_LOOPING, &b);
    return b;
}

float  ALSoundSource::getGain() {
    float g;
    alGetSourcef(source, AL_GAIN, &g);
    return g;
}

float  ALSoundSource::getReferenceDistance() {
    float d;
    alGetSourcef(source, AL_REFERENCE_DISTANCE, &d);
    return d;
}

float  ALSoundSource::getMinGain() {
    float g;
    alGetSourcef(source, AL_MIN_GAIN, &g);
    return g;
}

float  ALSoundSource::getMaxGain() {
    float g;
    alGetSourcef(source, AL_MAX_GAIN, &g);
    return g;
}

bool ALSoundSource::isPlaying() {
    ALenum s;
    if (audible) {
        alGetSourcei(source, AL_SOURCE_STATE, &s);
    } else {
        s = state;
    }
    return s == AL_PLAYING;
}

bool ALSoundSource::isPaused() {
    ALenum s;
    if (audible) {
        alGetSourcei(source, AL_SOURCE_STATE, &s);
    } else {
        s = state;
    }
    return s == AL_PAUSED;
}

float ALSoundSource::getCurrentOffsetInSecs() {
    float ofs;
    if (audible)
        alGetSourcef(source, AL_SEC_OFFSET, &ofs);
    else
        ofs = cached_offset;
    alCheck();
    return ofs;
}

float ALSoundSource::getEffectiveGain() {
    // perform effective gain calculation for OpenAL's
    // INVERSE_DISTANCE model
    float dist = (soundman->getListenerPosition() - getPosition()).length();
    float refdist = getReferenceDistance();
    float rolloff = 1.0f;
    float gain = getGain();
    return gain * refdist / (refdist + rolloff*(dist-refdist));
}

void ALSoundSource::setAudible(bool audible) {
    if (this->audible == audible) return;
    
    if (audible) {
        // making a source audible is only supported on OpenAL 1.1 or later
        if (soundman->openal_major  > 1 ||
            soundman->openal_major == 1 && soundman->openal_minor >= 1)
        {
            if      (state == AL_STOPPED) alSourceStop(source);
            else if (state == AL_PLAYING) alSourcePlay(source);
            else if (state == AL_PAUSED) alSourcePause(source);
            else if (state == AL_INITIAL) alSourceStop(source);

            alSourcef(source, AL_SEC_OFFSET, cached_offset);
            this->audible = true;
        }
    } else {
        alGetSourcei(source, AL_SOURCE_STATE, &state);
        if (soundman->openal_major  > 1 ||
            soundman->openal_major == 1 && soundman->openal_minor >= 1)
        {
            alGetSourcef(source, AL_SEC_OFFSET, &cached_offset);
        }
        alSourcePause(source);
        this->audible = false;
    }
}

void ALSoundSource::update(float delta_t) {
    if (audible) {
        // Nothing to do
    } else if (isPlaying()) {
        float new_offset = getCurrentOffsetInSecs() + delta_t;
        float sound_length = getSound()->getLengthInSecs();
        //ls_message("Source %p:\n", this);
        //ls_message("Sound_length: %f\n", sound_length);
        //ls_message("old_offset: %f\n", getCurrentOffsetInSecs());
        float old_offset = cached_offset;
        //ls_message("old_offset: %f\n", old_offset);
        //ls_message("   delta_t: %f\n", delta_t);
        //ls_message("new_offset: %f\n", new_offset);
        if (isLooping()) {
            while (new_offset > sound_length) {
                new_offset -= sound_length;
                //ls_message("new_offset: %f\n", new_offset);
            }
        } else {
            if (new_offset > sound_length) {
                new_offset = 0.0f;
                //ls_message("new_offset: %f\n", new_offset);
                state = AL_STOPPED;
            }
        }
        cached_offset = new_offset;
        //ls_message("final offset: %f\n", getCurrentOffsetInSecs());
    }
}

static void check(ALCdevice *dev=NULL) {
    ALCenum error = alcGetError(dev);
    if (error != ALC_NO_ERROR) {
        ls_error("OpenAL (ALC) Error %d", error);
        const ALbyte * errtxt = alcGetString(NULL,error);
        error = alcGetError(dev);
        if (error == ALC_NO_ERROR) {
            ls_error(": %s \n", errtxt);
        } else {
            ls_error(". Additionally, an error %d occurred"
                     " while reading the error text.\n", error);
        }
    }
}

ALSoundMan::ALSoundMan(Ptr<IConfig> config)
:	sound_dir(config->query("SoundMan_sound_dir"))
,   playing_sources(0)
{
    ls_message("Initializing ALSoundMan... ");

    device = alcOpenDevice( NULL );
    check();
    if (device == NULL ) {
        throw std::runtime_error("ALSoundMan: Failed to initialize Sound subsystem.");
    } else {
        const ALCbyte * device_specifier =
            alcGetString(device, ALC_DEVICE_SPECIFIER);
        check(device);
        ls_message("  Using device \"%s\"\n", device_specifier);
    }
    
    context = alcCreateContext( device, NULL );
    check();
    if (context == NULL ) {
        throw std::runtime_error("ALSoundman: Couldn't open audio context.");
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

ALSoundMan::~ALSoundMan() {
    alutExit();
    alcDestroyContext( (ALCcontext*) context );
    alcCloseDevice( device );
    ls_message("ALSoundMan shutdown complete.\n");
}

Ptr<Sound> ALSoundMan::querySound(const string & name) {
    if (sounds.find(name) == sounds.end())
        sounds[name] = new ALSound(sound_dir + "/" + name);
    return sounds[name];
}

Ptr<SoundSource> ALSoundMan::requestSource() {
    Ptr<ALSoundSource> s = new ALSoundSource(this);
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

bool invalid(WeakPtr<ALSoundSource> s) { return !s.valid(); }
bool playing(WeakPtr<ALSoundSource> s) {
    Ptr<ALSoundSource> p = s.lock();
    return p && p->isAudible() && p->isPlaying();
}
bool not_playing(Ptr<ALSoundSource> p) { return !p->isPlaying(); }

void ALSoundMan::update(float delta_t) {
    typedef vector<WeakPtr<ALSoundSource> >::iterator WSI;
    typedef vector<Ptr<ALSoundSource> >::iterator PSI;

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
        ALSoundSource *source = ptr(*i);
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
        ALSoundSource *source = ptr(*i);
        source->update(delta_t);
    }
}            

void ALSoundMan::manage(Ptr<SoundSource> src) {
	managed_sources.push_back(src);
}

void ALSoundMan::setListenerPosition( const Vector & pos ) {
    alListener3f( AL_POSITION, pos[0], pos[1], pos[2]);
}

void ALSoundMan::setListenerVelocity( const Vector & vel ) {
    alListener3f( AL_VELOCITY, vel[0], vel[1], vel[2]);
}

void ALSoundMan::setListenerOrientation(
        const Vector & up, const Vector & front ) {
    float upfront[6];
    
    for (int i=0; i<3; i++) upfront[i] = -front[i];
    for (int i=3; i<6; i++) upfront[i] = up[i-3];
    
    alListenerfv( AL_ORIENTATION, upfront);
}

Vector ALSoundMan::getListenerPosition() {
    float vec[3];
    alGetListenerfv(AL_POSITION, vec);
    return Vector(vec[0],vec[1],vec[2]);
}

Vector ALSoundMan::getListenerVelocity() {
    float vec[3];
    alGetListenerfv(AL_VELOCITY, vec);
    return Vector(vec[0],vec[1],vec[2]);
}

void ALSoundMan::getListenerOrientation(Vector & up, Vector & front) {
    float vec[6];
    alGetListenerfv(AL_ORIENTATION, vec);
    up    = Vector(vec[0],vec[1],vec[2]);
    front = Vector(vec[3],vec[4],vec[5]);
}

void ALSoundMan::flush() {
    sounds.clear();
}

void ALSoundMan::shutdown() {
	typedef vector<Ptr<ALSoundSource> > Sources;
	typedef Sources::iterator SIter;
	
	flush();
}	
