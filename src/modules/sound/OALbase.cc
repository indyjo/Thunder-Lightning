#include <AL/al.h>
#include <debug.h>

bool alCheck(const char *where) {
    ALenum error = alGetError();
    if (error == AL_NO_ERROR) {
        return true;
    } else {
        ls_error("OpenAL Error %d", error);
        const ALbyte * errtxt = alGetString(error);
        error = alGetError();
        if (error == AL_NO_ERROR) {
            ls_error(": %s \n", errtxt);
        } else {
            ls_error(". Additionally, an error %d occurred"
                     " while reading the error text.\n", error);
        }
        if (where) {
            ls_error("The error was detected %s\n", where);
        }
        return false;
    }
}

