#include "genericClass.h"
#include "model.h"
#include "intClass.h"
#include "compat.h"
#include "eval.h"
#include <string.h>
#include "garbageCollector.h"
#include "StringClass.h"
#include "arrayClass.h"
#include "arrayReferenceClass.h"

Variable_t* copyVariableToPtr(Variable_t var) {
	Variable_t* a = malloc(sizeof(Variable_t));
	*a = var;
	addPendingReference(a);
	return a;
}

MemberGetters_t memberGetters[] = {
	{IntClass, getIntegerMember},
	{StringClass, getStringMember},
	{IntArrayClass, getArrayMember},
	{StringArrayClass, getArrayMember},
	{ByteArrayClass, getArrayMember},
	{SolvedArrayReferenceClass, getArrayReferenceMember},
};



Variable_t* genericGet(Variable_t* var, CallArgs_t* ref) {
	if (ref->extraAction == ActionExtraMemberName) {
		for (u32 i = 0; i < ARRAY_SIZE(memberGetters); i++) {
			if (var->variableType == memberGetters[i].classType)
				return memberGetters[i].func(var, ref->extra);
		}
	}
	else if (ref->extraAction == ActionExtraArrayIndex) {
		Function_t* idx = ref->extra;
		Variable_t *solvedIdx = eval(idx->operations.data, idx->operations.count, 1);
		removePendingReference(solvedIdx);
		if (solvedIdx->variableType != IntClass)
			return NULL;

		return callMemberFunctionDirect(var, "get", &solvedIdx);
	}

	return NULL;
}

Variable_t* genericCallDirect(Variable_t* var, Variable_t** args, u8 len) {
	if (var->variableType != FunctionClass)
		return NULL;

	if (var->function.builtIn) {
		for (u32 i = 0; i < var->function.len; i++) {
			if (var->function.builtInPtr[i].argCount == len) {
				int valid = 1;
				if (var->function.builtInPtr[i].argCount != VARARGCOUNT) {
					for (u32 j = 0; j < var->function.builtInPtr[i].argCount; j++) {
						if (var->function.builtInPtr[i].argTypes[j] != args[j]->variableType || var->function.builtInPtr[i].argTypes[j] == VARARGCOUNT) {
							valid = 0;
							break;
						}
					}
				}

				if (valid) {
					return var->function.builtInPtr[i].func(var->function.origin, args, len);
				}
			}
		}
	}
	else {
		eval(var->function.function.operations.data, var->function.function.operations.count, 0);
		return &emptyClass;
	}

	return NULL;
}

Variable_t* genericCall(Variable_t* var, CallArgs_t* ref) {
	if (var->variableType != FunctionClass)
		return NULL;

	if (var->function.builtIn) {
		// TODO: implement arg handling

		Function_t* f = ref->extra;
		if (f->operations.count == 0) {
			return genericCallDirect(var, NULL, 0);
		}
		else {
			Vector_t argsHolder = newVec(sizeof(Variable_t*), 1);
			int lasti = 0;
			Operator_t* ops = f->operations.data;

			// Loops trough the function to get all args out
			for (int i = 0; i < f->operations.count; i++) {
				if (ops[i].token == EquationSeperator || i + 1 == f->operations.count) {
					if (i + 1 == f->operations.count)
						i++;

					Variable_t* var = eval(&ops[lasti], i - lasti, 1);
					if (var == NULL)
						return NULL; // maybe free first?

					lasti = i;
					vecAdd(&argsHolder, var);
				}
			}

			Variable_t *res = genericCallDirect(var, argsHolder.data, argsHolder.count);

			vecForEach(Variable_t*, tofree, (&argsHolder))
				removePendingReference(tofree);

			return res;
		}
	}
	else {
		eval(var->function.function.operations.data, var->function.function.operations.count, 0);
		return &emptyClass;
	}
}

Variable_t* getGenericFunctionMember(Variable_t* var, char* memberName, ClassFunctionTableEntry_t* entries, u8 len) {
	Variable_t newVar = {.readOnly = 1, .variableType = FunctionClass};
	newVar.function.origin = var;
	newVar.function.builtIn = 1;
	for (u32 i = 0; i < len; i++) {
		if (!strcmp(entries[i].name, memberName)) {
			newVar.function.builtInPtr = &entries[i];
			
			u32 j = i;
			for (; j < len && !strcmp(entries[j].name, memberName); j++);
			newVar.function.len = j - i;

			addPendingReference(var); // So caller doesn't fall out of scope. Don't forget to free!
			return copyVariableToPtr(newVar);
		}
	}

	return NULL;
}

Variable_t* callMemberFunctionDirect(Variable_t* var, char* memberName, Variable_t** other) {
	for (u32 i = 0; i < ARRAY_SIZE(memberGetters); i++) {
		if (var->variableType == memberGetters[i].classType) {
			Variable_t* funcRef = memberGetters[i].func(var, memberName);
			if (funcRef == NULL)
				return;

			Variable_t* callRes = genericCallDirect(funcRef, other, 1);
			removePendingReference(funcRef);
			return callRes;
		}
	}

	return NULL;
}

void freeVariable(Variable_t** target) {
	// Add specific freeing logic here
	Variable_t* referencedTarget = *target;
	
	if (!referencedTarget->reference) {
		switch (referencedTarget->variableType) {
			case StringClass:
			if (referencedTarget->string.free)
				FREE(referencedTarget->string.value);
			break;
			case StringArrayClass:
				vecForEach(char**, stringsInArray, (&referencedTarget->solvedArray.vector))
					FREE(*stringsInArray);
			case ByteArrayClass:
			case IntArrayClass:
				vecFree(referencedTarget->solvedArray.vector);
				break;
		}
	}
	

	FREE(referencedTarget);
	*target = NULL;
}