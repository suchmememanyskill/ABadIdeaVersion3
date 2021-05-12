#pragma once
#include "model.h"
#include "genericClass.h"
#include "vector.h"

Variable_t* stringFunctionHandler(char* funcName, Variable_t* caller, VariableReference_t* reference, Vector_t* args);
//Variable_t* freeStringVariable(Variable_t* in);
StringClass_t* createStringClassPtr(char* in, u8 free);
StringClass_t createStringClass(char* in, u8 free);
char* getStringValue(Variable_t* var);

Variable_t newStringVariable(char *x, u8 readOnly, u8 freeOnExit);
#define newStringVariablePtr(x, readOnly, freeOnExit) copyVariableToPtr(newStringVariable(x, readOnly, freeOnExit))