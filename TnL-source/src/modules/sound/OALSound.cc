#include <AL/al.h>
#include <AL/alut.h>
#include <debug.h>
#include "OALbase.h"
#include "OALSound.h"

OALSound::OALSound( const string & filename ) {
    ls_message("OALSound::OALSound('%s')\n", filename.c_str());
    alCheck("before creating a buffer");
    buffer = alutCreateBufferFromFile(filename.c_str());
    alCheck("after creating a buffer");
}

OALSound::~OALSound() {
    alCheck("before deleting buffer");
    alDeleteBuffers( 1, &buffer );
    alCheck("after deleting buffer");
}

float Sound::getLengthInSecs() {
    ALint size, bits, freq;
    alCheck("before calculating buffer length");
    alGetBufferi(buffer, AL_SIZE, &size);
    alGetBufferi(buffer, AL_BITS, &bits);
    alGetBufferi(buffer, AL_FREQUENCY, &freq);
    alCheck("after calculating buffer length");
    
    return size / (bits/8) / (float) freq;
}

