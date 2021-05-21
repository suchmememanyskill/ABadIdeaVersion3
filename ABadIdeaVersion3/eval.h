#pragma once
#include "model.h"

Variable_t* eval(Operator_t* ops, u32 len, u8 ret);
void setStaticVars(Vector_t* vec);