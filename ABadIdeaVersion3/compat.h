#pragma once

#ifdef WIN32
	#include <stdio.h>
	#define gfx_printf(str, ...) printf(str, __VA_ARGS__)
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
	#define LPVERSION_MAJOR 3
	#define LPVERSION_MINOR 0
	#define LPVERSION_BUGFX 5
	#define FREE(x) if (x) free(x)
#endif