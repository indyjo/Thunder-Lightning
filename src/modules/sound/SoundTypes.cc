#include "OALSoundManager.h"

const Type Sound::type("Sound", 0);
const Type SoundSource::type("SoundSource", 0);
const Type SoundMan::type("SoundMan", 0);

const Type ProxySoundSource::type("ProxySoundSource", SoundSource::type, 0);
const Type VirtualSoundSource::type("VirtualSoundSource", SoundSource::type, 0);

const Type OALSound::type("OALSound", Sound::type, 0);
const Type OALSoundSource::type("OALSoundSource", SoundSource::type, 0);
const Type OALSoundMan::type("OALSoundMan", SoundMan::type, 0);

const Type OAL10SoundSource::type("OAL10SoundSource", OALSoundSource::type, 0);
const Type OAL11SoundSource::type("OAL11SoundSource", OALSoundSource::type, 0);

