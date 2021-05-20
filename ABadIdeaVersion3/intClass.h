#pragma once
#include "model.h"
#include "genericClass.h"
#include "vector.h"



s64 getIntValue(Variable_t* var);

IntClass_t createIntClass(s64 in);
IntClass_t* createIntClassPtr(s64 in);

Variable_t* intFunctionHandler(char* funcName, Variable_t* caller, VariableReference_t* reference, Vector_t* args);

Variable_t newIntVariable(s64 x, u8 readOnly);
#define newIntVariablePtr(x, readOnly) copyVariableToPtr(newIntVariable(x, readOnly))
Variable_t* getIntegerMember(Variable_t* var, char* memberName);