#pragma once
#include "model.h"

Variable_t* eval(Operator_t* ops, u32 len, u8 ret);
void setStaticVars(Vector_t* vec);
void initRuntimeVars();
void exitRuntimeVars();
void runtimeVariableEdit(Callback_SetVar_t* set, Variable_t* curRes);