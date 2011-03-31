#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>

void ls_message(const char *fmt, ...);
void ls_warning(const char *fmt, ...);
void ls_error(const char *fmt, ...);

void debug(int severity, const char *fmt, va_list ap);

#endif
