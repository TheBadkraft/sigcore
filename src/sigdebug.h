#ifndef SIGDEBUG_H
#define SIGDEBUG_H

#include "sigcore.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef TSTDBG
	#define DEBUG_PRINT(stream, fmt, ...) \
		fprintf(stream, "[DEBUG] " fmt, ##__VA_ARGS__); \
		fprintf(stream, "\n");
#else
	#define DEBUG_PRINT(stream, fmt, ...) \
		do { } while (0)  /* No-op when TSTDBG is not defined */
#endif

#endif /* SIGDEBUG_H */
