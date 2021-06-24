#pragma once
#include "model.h"

enum {
	SCRIPT_FATAL = 0,
	SCRIPT_PARSER_FATAL,
	SCRIPT_WARN,
};

extern s64 scriptCurrentLine;

void printScriptError(u8 errLevel, char* message, ...);

#ifdef WIN32
#define SCRIPT_FATAL_ERR(message, ...) printScriptError(SCRIPT_FATAL, message, __VA_ARGS__); return NULL
#define SCRIPT_WARN_ERR(message, ...) printScriptError(SCRIPT_WARN, message, __VA_ARGS__)
#else
#define SCRIPT_FATAL_ERR(message, ...) printScriptError(SCRIPT_FATAL, message, ##__VA_ARGS__); return NULL
#define SCRIPT_WARN_ERR(message, ...) printScriptError(SCRIPT_WARN, message, ##__VA_ARGS__)
#endif // WIN32
