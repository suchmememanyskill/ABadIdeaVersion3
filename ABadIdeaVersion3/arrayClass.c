#include "model.h"
#include "compat.h"
#include "genericClass.h"
#include "intClass.h"
#include "arrayClass.h"
#include "garbageCollector.h"
#include "eval.h"
#include "scriptError.h"
#include "StringClass.h"

u8 anotherOneIntArg[] = { IntClass };
u8 oneStringoneFunction[] = { StringClass, FunctionClass };
u8 oneIntOneAny[] = { IntClass, VARARGCOUNT };

ClassFunction(getArrayIdx) {
	s64 getVal = (*args)->integer.value;
	// Out of bounds
	if (getVal < 0 || getVal >= caller->solvedArray.vector.count) {
		SCRIPT_FATAL_ERR("Accessing index %d while array is %d long", (int)getVal, (int)caller->solvedArray.vector.count);
	}

	if (caller->variableType == IntArrayClass) {
		s64* arr = caller->solvedArray.vector.data;
		return copyVariableToPtr(newIntVariable(arr[getVal]));
	}
	else if (caller->variableType == StringArrayClass) {
		char** arr = caller->solvedArray.vector.data;
		Variable_t v = newStringVariable(arr[getVal], 1, 0);
		v.readOnly = 1;
		v.reference = 1;
		return copyVariableToPtr(v);
	}
	else if (caller->variableType == ByteArrayClass) {
		u8* arr = caller->solvedArray.vector.data;
		return copyVariableToPtr(newIntVariable(arr[getVal]));
	}

	return NULL;
}

ClassFunction(getArrayLen) {
	return newIntVariablePtr(caller->solvedArray.vector.count);
}

ClassFunction(createRefSkip) {
	s64 skipAmount = getIntValue(*args);
	if (caller->solvedArray.vector.count < skipAmount || skipAmount <= 0) {
		SCRIPT_FATAL_ERR("Accessing index %d while array is %d long", (int)skipAmount, (int)caller->solvedArray.vector.count);
	}
		
	Variable_t refSkip = { .variableType = SolvedArrayReferenceClass };
	refSkip.solvedArray.arrayClassReference = caller;
	refSkip.solvedArray.offset = skipAmount;
	refSkip.solvedArray.len = caller->solvedArray.vector.count - skipAmount;
	addPendingReference(caller);
	return copyVariableToPtr(refSkip);
}

ClassFunction(arrayForEach) {
	Vector_t* v = &caller->solvedArray.vector;

	Callback_SetVar_t setVar = { .isTopLevel = 1, .varName = (*args)->string.value };
	Variable_t* iter = NULL;
	if (caller->variableType == IntArrayClass)
		iter = copyVariableToPtr(newIntVariable(0));

	runtimeVariableEdit(&setVar, iter);

	for (int i = 0; i < v->count; i++) {
		if (caller->variableType == IntArrayClass) {
			s64* arr = v->data;
			iter->integer.value = arr[i];
		}
		else if (caller->variableType == StringArrayClass) {
			char** arr = caller->solvedArray.vector.data;
			Variable_t v = newStringVariable(arr[i], 1, 0);
			v.readOnly = 1;
			v.reference = 1;
			*iter = v;
		}
		else if (caller->variableType == ByteArrayClass) {
			u8* arr = v->data;
			iter->integer.value = arr[i];
		}

		Variable_t* res = genericCallDirect(args[1], NULL, 0);
		if (res == NULL)
			return NULL;
	}

	return &emptyClass;
;}

ClassFunction(arrayCopy) {
	Vector_t* v = &caller->solvedArray.vector;
	Vector_t copiedArray = vecCopy(v);
	Variable_t var = { .variableType = caller->variableType, .solvedArray.vector = copiedArray };
	return copyVariableToPtr(var);
}

ClassFunction(arraySet) {
	s64 idx = getIntValue(*args);
	Vector_t* v = &caller->solvedArray.vector;
	if (v->count < idx || idx <= 0) {
		SCRIPT_FATAL_ERR("Accessing index %d while array is %d long", (int)idx, (int)caller->solvedArray.vector.count);
	}

	if (caller->variableType == IntArrayClass) {
		if (args[1]->variableType != IntClass) {
			return NULL; // TODO: add proper error handling
		}

		s64* a = v->data;
		a[idx] = getIntValue(args[1]);
	}
	else if (caller->variableType == StringArrayClass) {
		if (args[1]->variableType != StringClass) {
			return NULL; // TODO: add proper error handling
		}

		char** a = v->data;
		FREE(a[idx]);
		a[idx] = CpyStr(args[1]->string.value);
	}
	else if (caller->variableType == ByteArrayClass) {
		if (args[1]->variableType != IntClass) {
			return NULL; // TODO: add proper error handling
		}

		u8* a = v->data;
		a[idx] = (u8)(getIntValue(args[1]) & 0xFF);
	}

	return &emptyClass;
}

ClassFunctionTableEntry_t arrayFunctions[] = {
	{"get", getArrayIdx, 1, anotherOneIntArg },
	{"len", getArrayLen, 0, 0},
	{"skip", createRefSkip, 1, anotherOneIntArg},
	{"foreach", arrayForEach, 2, oneStringoneFunction},
	{"copy", arrayCopy, 0, 0},
	{"set", arraySet, 2, oneIntOneAny},
};

Variable_t getArrayMember(Variable_t* var, char* memberName) {
	return getGenericFunctionMember(var, memberName, arrayFunctions, ARRAY_SIZE(arrayFunctions));
}