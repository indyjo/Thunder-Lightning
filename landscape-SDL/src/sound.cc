#include <iostream>
#include <algorithm>

#include <landscape.h>
#include <interfaces/IConfig.h>
#include "sound.h"

#include <AL/alu.h>
#include <AL/alut.h>

using namespace std;


Sound::Sound( const string & filename ) {
    alGetError(); // reset error state
    alGenBuffers( 1, & buffer );
    
    ALsizei size, freq, bits;
    ALenum format;
    ALvoid *data;
    ALboolean loop, err;

    //err = alutLoadWAV(filename.c_str(), &data, &format, &size, &bits, &freq);
    /*
    if(err == AL_FALSE) {
        ls_error("Sound: Could not load %s\n", filename.c_str());
    }
    */
    #ifdef MACINTOSH_AL
    alutLoadWAVFile((ALbyte*)filename.c_str(), &format, &data, &size, &freq);
    #else
    alutLoadWAVFile((ALbyte*)filename.c_str(), &format, &data, &size, &freq, &loop);
    #endif

    alBufferData(buffer, format, data, size, freq);
    ALenum e;
    if ((e=alGetError())!=AL_NO_ERROR) {
        ls_error("Sound error: \n");
        switch(e) {
        //case AL_INVALID_OPERATION: ls_error("INVALID_OPERATION\n"); break;
        case AL_OUT_OF_MEMORY: ls_error("OUT_OF_MEMORY\n"); break;
        default: ls_error("UNKNOWN ERROR\n");
        }
    }
}

Sound::~Sound() {
    alDeleteBuffers( 1, &buffer );
}


SoundSource::SoundSource(Ptr<SoundMan> soundman)
:   soundman(soundman)
{
    alGenSources( 1, & source );
    setReferenceDistance(50);
}

SoundSource::~SoundSource() {
    if (isPlaying()) stop();
    alDeleteSources(1, & source );
}

void SoundSource::setPosition(const Vector & pos) {
    alSource3f(source, AL_POSITION, pos[0], pos[1], pos[2]);
}

void SoundSource::setVelocity(const Vector & vel) {
    alSource3f(source, AL_VELOCITY, vel[0], vel[1], vel[2]);
}

void SoundSource::setLooping(bool loop) {
    alSourcei(source, AL_LOOPING, loop?AL_TRUE:AL_FALSE);
}

void SoundSource::setPitch(float pitch) {
    alSourcef(source, AL_PITCH, pitch);
}

void SoundSource::setGain(float gain) {
    alSourcef(source, AL_GAIN, gain );
}

void SoundSource::setReferenceDistance(float dist) {
    alSourcef(source, AL_REFERENCE_DISTANCE, dist);
}

void SoundSource::play(Ptr<Sound> sound) {
    if (!soundman->requestPlayChannel(this)) return;
    ALuint buffer = sound->getResource();
    alSourceQueueBuffers( source, 1, & buffer );
    alSourcePlay(source);
}

void SoundSource::pause() {
    alSourcePause(source);
}

void SoundSource::stop() {
    alSourceStop(source);
    soundman->update();
}

void SoundSource::rewind() {
    alSourceRewind(source);
}

bool SoundSource::isPlaying() {
    ALenum state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

SoundMan::SoundMan(Ptr<IConfig> config)
:	sound_dir(config->query("SoundMan_sound_dir"))
{
    device = alcOpenDevice( NULL );
    if (device == NULL ) ls_error("Soundman: Couldn't open audio device.");
    context = alcCreateContext( device, NULL );
    if (context == NULL ) ls_error("Soundman: Couldn't open audio context.");
    alcMakeContextCurrent( (ALCcontext*) context );
    
    alDopplerVelocity(config->queryFloat("SoundMan_doppler_velocity",10000.0));
    alDopplerFactor(config->queryFloat("SoundMan_doppler_factor", 1));
    play_channels = config->queryInt("SoundMan_channels", 32);
}

SoundMan::~SoundMan() {
    alcDestroyContext( (ALCcontext*) context );
    alcCloseDevice( device );
}

Ptr<Sound> SoundMan::querySound(const string & name) {
    if (sounds.find(name) == sounds.end())
        sounds[name] = new Sound(sound_dir + "/" + name);
    return sounds[name];
}

Ptr<SoundSource> SoundMan::requestSource() {
    return new SoundSource(this);
}

bool SoundMan::requestPlayChannel(SoundSource *source) {
    if (play_channels == 0) return false;
    play_channels--;
    playing_sources.push_back(source);
    return true;
}

bool not_playing(SoundSource* s) { return !s->isPlaying(); }
bool not_playing_ptr(Ptr<SoundSource> s) { return !s->isPlaying(); }

// collects play channels from sound sources which are no longer playing
void SoundMan::update() {
    typedef vector<SoundSource*>::iterator SI;
    SI new_end = remove_if(playing_sources.begin(),
                           playing_sources.end(),
                           not_playing);
    playing_sources.resize(new_end - playing_sources.begin());
    play_channels = playing_sources.size();
    
    managed_sources.resize(
    	remove_if(managed_sources.begin(), managed_sources.end(), not_playing_ptr)
    	- managed_sources.begin());
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

void SoundMan::flush() {
    sounds.clear();
}

void SoundMan::shutdown() {
	typedef vector<Ptr<SoundSource> > Sources;
	typedef Sources::iterator SIter;
	
	flush();
	
	playing_sources.clear();
}	
