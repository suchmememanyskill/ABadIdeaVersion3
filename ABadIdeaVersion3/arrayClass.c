#include "model.h"
#include "compat.h"
#include "genericClass.h"
#include "intClass.h"
#include "arrayClass.h"

u8 anotherOneIntArg[] = { IntClass };

ClassFunction(getArrayIdx) {
	if (caller->variableType == IntArrayClass) {
		s64 getVal = (*args)->integer.value;
		// Out of bounds
		if (getVal < 0 || getVal >= caller->solvedArray.vector.count)
			return NULL;

		s64* arr = caller->solvedArray.vector.data;
		return copyVariableToPtr(newIntVariable(arr[getVal], 0));
	}

	return NULL;
}

ClassFunctionTableEntry_t arrayFunctions[] = {
	{"get", getArrayIdx, 1, anotherOneIntArg },
};

Variable_t* getArrayMember(Variable_t* var, char* memberName) {
	return getGenericFunctionMember(var, memberName, arrayFunctions, ARRAY_SIZE(arrayFunctions));
}