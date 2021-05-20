#include "intClass.h"
#include "compat.h"
#include "vector.h"
#include <malloc.h>
#include <string.h>

s64 getIntValue(Variable_t* var) {
	if (var->variableType != IntClass)
		return 0;

	return var->integer.value;
}

IntClass_t createIntClass(s64 in) {
	IntClass_t a = { in };
	return a;
}

IntClass_t* createIntClassPtr(s64 in) {
	IntClass_t* a = malloc(sizeof(IntClass_t));
	*a = createIntClass(in);
	return a;
}
/*
Variable_t* freeIntVariable(Variable_t* in) {
	free(in->variable);
	return NULL;
}
*/

Variable_t newIntVariable(s64 x, u8 readOnly) {
	Variable_t var = { .variableType = IntClass, .readOnly = readOnly, .integer = createIntClass(x) };
	return var;
}

ClassFunction(printIntVariable) {
	if (caller->variableType == IntClass) {
		IntClass_t* a = &caller->integer;
		gfx_printf("%d", a->value);
	}
	return NULL;
}

ClassFunction(addIntVariables) {
	if (!args)
		return NULL;

	Variable_t* a = *args;
	if (a->variableType != IntClass)
		return NULL;

	s64 i1 = getIntValue(caller);
	s64 i2 = getIntValue(a);

	return newIntVariablePtr((i1 + i2), 0);
}

u8 oneVarArg[] = { VARARGCOUNT };
u8 oneIntArg[] = { IntClass };

ClassFunctionTableEntry_t intFunctions[] = {
	{"__print__", printIntVariable, 0, 0},
	{"+", addIntVariables, 1, oneIntArg },
};

Variable_t* getIntegerMember(Variable_t* var, char* memberName) {
	return getGenericFunctionMember(var, memberName, intFunctions, ARRAY_SIZE(intFunctions));
}

Variable_t* intFunctionHandler(char* funcName, Variable_t* caller, VariableReference_t* reference, Vector_t* args) {
	// TODO: implement arg count detection
	for (u32 i = 0; i < ARRAY_SIZE(intFunctions); i++) {
		if (!strcmp(funcName, intFunctions[i].name)) {
			return intFunctions[i].func(caller, reference, args);
		}
	}

	gfx_printf("[FATAL] function %s not found", funcName);
	return NULL;
}