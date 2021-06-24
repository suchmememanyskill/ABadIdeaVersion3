#include "intClass.h"
#include "compat.h"
#include <malloc.h>
#include <string.h>

IntClass_t createIntClass(s64 in) {
	IntClass_t a = { in };
	return a;
}

Variable_t newIntVariable(s64 x) {
	// Integers are always read-only
	Variable_t var = { .variableType = IntClass, .readOnly = 1, .integer = createIntClass(x) };
	return var;
}

ClassFunction(printIntVariable) {
	if (caller->variableType == IntClass) {
		IntClass_t* a = &caller->integer;
		gfx_printf("%lld", a->value);
	}
	return &emptyClass;
}

ClassFunction(addIntVariables) {
	s64 i1 = getIntValue(caller);
	s64 i2 = getIntValue(*args);

	return newIntVariablePtr((i1 + i2));
}

ClassFunction(minusIntVariables) {
	s64 i1 = getIntValue(caller);
	s64 i2 = getIntValue(*args);

	return newIntVariablePtr((i1 - i2));
}

ClassFunction(multiplyIntVariables) {
	s64 i1 = getIntValue(caller);
	s64 i2 = getIntValue(*args);

	return newIntVariablePtr((i1 * i2));
}

ClassFunction(equalIntVariables) {
	s64 i1 = getIntValue(caller);
	s64 i2 = getIntValue(*args);

	return newIntVariablePtr((i1 == i2));
}

ClassFunction(notEqualIntVariables) {
	s64 i1 = getIntValue(caller);
	s64 i2 = getIntValue(*args);

	return newIntVariablePtr((i1 != i2));
}

ClassFunction(smallerIntVariables) {
	s64 i1 = getIntValue(caller);
	s64 i2 = getIntValue(*args);

	return newIntVariablePtr((i1 < i2));
}

ClassFunction(notInt) {
	return newIntVariablePtr(!(getIntValue(caller)));
}

u8 oneVarArg[] = { VARARGCOUNT };
u8 oneIntArg[] = { IntClass };

ClassFunctionTableEntry_t intFunctions[] = {
	{"print", printIntVariable, 0, 0},
	{"+", addIntVariables, 1, oneIntArg },
	{"-", minusIntVariables, 1, oneIntArg },
	{"*", multiplyIntVariables, 1, oneIntArg },
	{"==", equalIntVariables, 1, oneIntArg },
	{"!=", notEqualIntVariables, 1, oneIntArg },
	{"<", smallerIntVariables, 1, oneIntArg},
	{"not", notInt, 0, 0},
};

Variable_t getIntegerMember(Variable_t* var, char* memberName) {
	return getGenericFunctionMember(var, memberName, intFunctions, ARRAY_SIZE(intFunctions));
}