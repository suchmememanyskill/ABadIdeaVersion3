#include "StringClass.h"
#include "compat.h"
#include "vector.h"
#include <malloc.h>

char* getStringValue(Variable_t* var) {
	if (var->variableType != StringClass)
		return NULL;

	return var->string.value;
}

// Will NOT copy the string, the pointer is taken as-is
StringClass_t createStringClass(char* in, u8 free) {
	StringClass_t a = { 0 };
	a.free = free;
	a.value = in;
	return a;
}

Variable_t newStringVariable(char *x, u8 readOnly, u8 freeOnExit) {
	Variable_t var = { .variableType = StringClass, .readOnly = readOnly, .string = createStringClass(x, freeOnExit) };
	return var;
}

ClassFunction(printStringVariable) {
	if (caller->variableType == StringClass) {
		StringClass_t* a = &caller->string;
		gfx_printf("%s", a->value);
	}
	return NULL;
}

ClassFunctionTableEntry_t stringFunctions[] = {
	{"__print__", printStringVariable, 0, 0},
};