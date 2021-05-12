#pragma once

#ifdef WIN32
	#include <stdio.h>
	#define gfx_printf(str, ...) printf(str, __VA_ARGS__)
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif