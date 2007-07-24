#include <stdexcept>
#include "OALbase.h"
#include "OALSoundSource.h"


namespace {

    inline bool CHECKFLOAT(float f, bool quiet=false) {
    #ifdef NDEBUG
        if ((f) != (f) || 2*(f) == (f) && (f) != 0) {
            if (!quiet) ls_warning("Invalid float value detected (%f): ignored.\n", f);
            return false;
        }
    #else
        if ((f) != (f) || 2*(f) == (f) && (f) != 0) {
            *(char*)0x0 = 'X';
            return false;
        }
    #endif
        return true;
    }

    inline void CHECKVECTOR(const Vector & v, const char *where=0) {
        if (!CHECKFLOAT(v[0], true) ||
            !CHECKFLOAT(v[1], true) ||
            !CHECKFLOAT(v[2], true))
        {
            if (where) {
                ls_error("Illegal vector %s: ", where); v.dump();
            } else {
                ls_error("Illegal vector: "); v.dump()
            }
        }
    }

} // namespace






////////////////////////////////////////////////////////////////////////////////
// OALSoundSource implementation
////////////////////////////////////////////////////////////////////////////////


OALSoundSource::OALSoundSource()
    : TypedObject(type)
{
    alCheck("before generating source");
    alGenSources( 1, & source );
    alCheck("after generating source");
    setReferenceDistance(50);
}

OALSoundSource::~OALSoundSource() {
    alCheck("before deleting source");
    if (isPlaying()) stop();
    alDeleteSources(1, & source );
    alCheck("after deleting source");
}

Vector OALSoundSource::getPosition() {
    float vec[3];
    alGetSourcefv(source, AL_POSITION, vec);
    return Vector(vec[0], vec[1], vec[2]);
}

void OALSoundSource::setPosition(const Vector & pos) {
    alSource3f(source, AL_POSITION, pos[0], pos[1], pos[2]);
    alCheck("after setting position");
    CHECKVECTOR(pos, "position");
}

Vector OALSoundSource::getVelocity() {
    float vec[3];
    alGetSourcefv(source, AL_VELOCITY, vec);
    alCheck("after setting velocity");
    return Vector(vec[0], vec[1], vec[2]);
}

void OALSoundSource::setVelocity(const Vector & vel) {
    alSource3f(source, AL_VELOCITY, vel[0], vel[1], vel[2]);
    CHECKVECTOR(vel, "velocity");
}

bool OALSoundSource::isLooping() {
    ALint b;
    alGetSourcei(source, AL_LOOPING, &b);
    return b;
}

void OALSoundSource::setLooping(bool loop) {
    alSourcei(source, AL_LOOPING, loop?AL_TRUE:AL_FALSE);
}

float OALSoundSource::getPitch() {
    float pitch;
    alGetSourcef(source, AL_PITCH, &pitch);
    return pitch;
}

void OALSoundSource::setPitch(float pitch) {
    alSourcef(source, AL_PITCH, pitch);
}

float OALSoundSource::getGain() {
    float gain;
    alGetSourcef(source, AL_GAIN, &gain);
    return gain;
}

void OALSoundSource::setGain(float gain) {
    alSourcef(source, AL_GAIN, gain );
}

float OALSoundSource::getReferenceDistance() {
    float dist;
    alGetSourcef(source, AL_REFERENCE_DISTANCE, &dist);
    return dist;
}

void OALSoundSource::setReferenceDistance(float dist) {
    alSourcef(source, AL_REFERENCE_DISTANCE, dist);
}

float OALSoundSource::getMinGain() {
    float g;
    alGetSourcef(source, AL_MIN_GAIN, &g);
    return g;
}

void OALSoundSource::setMinGain(float gain) {
    alSourcef(source, AL_MIN_GAIN, gain );
}

float OALSoundSource::getMaxGain() {
    float g;
    alGetSourcef(source, AL_MAX_GAIN, &g);
    return g;
}

void OALSoundSource::setMaxGain(float gain) {
    alSourcef(source, AL_MAX_GAIN, gain );
}

void OALSoundSource::play(Ptr<Sound> sound) {
    if (! sound->isA(OALSound::type)) {
        ls_error("Wrong type of sound passed to OALSoundSource.");
        throw runtime_error("Wrong type of sound passed to OALSoundSource.");
    }
    this->sound = sound;
    ALuint buffer = sound->getResource();
    alSourceQueueBuffers( source, 1, & buffer );
    alSourcePlay(source);
}

void OALSoundSource::pause() {
    alSourcePause(source);
}

void OALSoundSource::stop() {
    alSourceStop(source);
}

void OALSoundSource::rewind() {
    alSourceRewind(source);
}

bool OALSoundSource::isPlaying() {
    ALenum s;
    alGetSourcei(source, AL_SOURCE_STATE, &s);
    return s == AL_PLAYING;
}

bool OALSoundSource::isPaused() {
    ALenum s;
    alGetSourcei(source, AL_SOURCE_STATE, &s);
    return s == AL_PAUSED;
}

float OALSoundSource::getEffectiveGain(const Vector & listener) {
    // perform effective gain calculation for OpenAL's
    // INVERSE_DISTANCE model
    float dist = (listener - getPosition()).length();
    float refdist = getReferenceDistance();
    float rolloff = 1.0f;
    float gain = getGain();
    return gain * refdist / (refdist + rolloff*(dist-refdist));
}


////////////////////////////////////////////////////////////////////////////////
// OAL10SoundSource implementation
////////////////////////////////////////////////////////////////////////////////


OAL10SoundSource::OAL10SoundSource
:   current_offset(0)
{
    setType(type);
}

OAL10Source::~OAL10SoundSource()
{ }

void OAL10SoundSource::update(float delta_t) {
    if (isPlaying()) {
        current_offset += delta_t;
    } else {
        current_offset = 0;
    }
}

float OAL10SoundSource::getCurrentOffsetInSecs() {
    return current_offset;
}


////////////////////////////////////////////////////////////////////////////////
// OAL11SoundSource implementation
////////////////////////////////////////////////////////////////////////////////


OAL11SoundSource::OAL11SoundSource
{
    setType(type);
}

OAL11SoundSource::~OAL11SoundSource()
{ }

float OAL11SoundSource::getCurrentOffsetInSecs() {
    float ofs;
    alGetSourcef(source, AL_SEC_OFFSET, &ofs);
    alCheck("after getting current offset");
    return ofs;
}

