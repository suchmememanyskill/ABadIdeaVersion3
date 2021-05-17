
#pragma once
#include "vector.h"

typedef struct {
	Function_t main;
	Vector_t staticVarHolder;
} ParserRet_t;

ParserRet_t parseScript(char* in);