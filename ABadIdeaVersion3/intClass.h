#pragma once
#include "model.h"
#include "genericClass.h"
#include "vector.h"



s64 getIntValue(Variable_t* var);

IntClass_t createIntClass(s64 in);

Variable_t newIntVariable(s64 x);
#define newIntVariablePtr(x, readOnly) copyVariableToPtr(newIntVariable(x))
#define newIntVariablePtr(x) copyVariableToPtr(newIntVariable(x))
Variable_t* getIntegerMember(Variable_t* var, char* memberName);