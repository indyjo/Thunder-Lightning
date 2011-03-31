#ifndef SOUND_OALBASE_H
#define SOUND_OALBASE_H

/// Check whether the last OpenAL call set an error. This will reset OpenAL's
/// error state.
/// @param where If non-zero, a human readable description of where an error
///        might have occurred. Example: "before context creation".
/// @return true  if there was no error,
///         false if there has been an error.
/// @note In case of an error, the user is warned with a debug message.
bool alCheck(const char *where=0);

/// Check whether the last ALC call set an error. This will reset ALC's
/// error state.
/// @param where If non-zero, a human readable description of where an error
///        might have occurred. Example: "before context creation".
/// @return true  if there was no error,
///         false if there has been an error.
/// @note In case of an error, the user is warned with a debug message.
bool alcCheck(const char *where=0);

#endif

