
#pragma once
#include "compat.h"

typedef struct {
	Function_t main;
	Vector_t staticVarHolder;
	u8 valid;
} ParserRet_t;

#ifdef WIN32
#define SCRIPT_PARSER_ERR(message, ...) printScriptError(SCRIPT_PARSER_FATAL, message, __VA_ARGS__); return (ParserRet_t){0}
#else
#define SCRIPT_PARSER_ERR(message, ...) printScriptError(SCRIPT_PARSER_FATAL, message, ##__VA_ARGS__); return (ParserRet_t){0}
#endif // WIN32

ParserRet_t parseScript(char* in);
void exitStaticVars(Vector_t* v);
void exitFunction(Operator_t* start, u32 len);