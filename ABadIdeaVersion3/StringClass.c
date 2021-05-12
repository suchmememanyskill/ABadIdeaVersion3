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

StringClass_t* createStringClassPtr(char* in, u8 free) {
	StringClass_t* a = malloc(sizeof(StringClass_t));
	*a = createStringClass(in, free);
	return a;
}

Variable_t newStringVariable(char *x, u8 readOnly, u8 freeOnExit) {
	Variable_t var = { .variableType = StringClass, .readOnly = readOnly, .string = createStringClass(x, freeOnExit) };
	return var;
}

/*
Variable_t* freeStringVariable(Variable_t* in) {
	if (in->variableType != StringClass) {
		StringClass_t* a = (StringClass_t*)in->variable;
		if (a->free)
			free(a->value);
	}
	free(in->variable);
}
*/

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

Variable_t* stringFunctionHandler(char* funcName, Variable_t* caller, VariableReference_t* reference, Vector_t* args) {
	// TODO: implement arg count detection
	for (u32 i = 0; i < ARRAY_SIZE(stringFunctions); i++) {
		if (!strcmp(funcName, stringFunctions[i].name)) {
			return stringFunctions[i].func(caller, reference, args);
		}
	}

	gfx_printf("[FATAL] function %s not found", funcName);
	return NULL;
}