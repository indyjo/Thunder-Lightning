#include <stdio.h>
#include "debug.h"

void ls_message(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	debug(0, fmt, ap);
	va_end(ap);
}

void ls_warning(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	debug(2, fmt, ap);
	va_end(ap);
}

void ls_error(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	debug(4, fmt, ap);
	va_end(ap);
}

#ifdef __MINGW32__
void debug(int severity, const char *fmt, va_list ap) {
	vprintf(fmt, ap);
	fflush(stdout);
}
#else
void debug(int severity, const char *fmt, va_list ap) {
	if (severity<2) {
  		printf("\033[0;32m");
	} else if (severity <4) {
  		printf("\033[0;33m");
	} else {
  		printf("\033[0;31m");
	}
	vprintf(fmt, ap);
	fflush(stdout);
	printf("\033[0;39m");
}
#endif
