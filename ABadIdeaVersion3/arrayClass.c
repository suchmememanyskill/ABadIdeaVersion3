#include "model.h"
#include "compat.h"
#include "genericClass.h"
#include "intClass.h"
#include "arrayClass.h"
#include "garbageCollector.h"

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

ClassFunction(getArrayLen) {
	return newIntVariablePtr(caller->solvedArray.vector.count);
}

ClassFunction(createRefSkip) {
	s64 skipAmount = getIntValue(*args);
	if (caller->solvedArray.vector.count < skipAmount || skipAmount <= 0)
		return NULL;

	Variable_t refSkip = { .variableType = SolvedArrayReferenceClass };
	refSkip.solvedArray.arrayClassReference = caller;
	refSkip.solvedArray.offset = skipAmount;
	refSkip.solvedArray.len = caller->solvedArray.vector.count - skipAmount;
	addPendingReference(caller);
	return copyVariableToPtr(refSkip);
}

ClassFunctionTableEntry_t arrayFunctions[] = {
	{"get", getArrayIdx, 1, anotherOneIntArg },
	{"len", getArrayLen, 0, 0},
	{"skip", createRefSkip, 1, anotherOneIntArg},
};

Variable_t* getArrayMember(Variable_t* var, char* memberName) {
	return getGenericFunctionMember(var, memberName, arrayFunctions, ARRAY_SIZE(arrayFunctions));
}