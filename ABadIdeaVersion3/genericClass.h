#pragma once
#include "model.h"

Variable_t* copyVariableToPtr(Variable_t var);

#define VARARGCOUNT 255

#define ClassFunction(name) Variable_t* name(Variable_t* caller, VariableReference_t* reference, Vector_t* args)