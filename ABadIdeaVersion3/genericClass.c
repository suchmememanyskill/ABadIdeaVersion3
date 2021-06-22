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
#include "functionClass.h"

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
			if (var->variableType == memberGetters[i].classType) {
				Variable_t member = memberGetters[i].func(var, ref->extra);
				if (member.variableType == None)
					return NULL;

				addPendingReference(var); // So caller doesn't fall out of scope. Don't forget to free!
				return copyVariableToPtr(member);
			}
		}
	}
	else if (ref->extraAction == ActionExtraArrayIndex) {
		Function_t* idx = ref->extra;
		Variable_t *solvedIdx = eval(idx->operations.data, idx->operations.count, 1);
		removePendingReference(solvedIdx);
		if (solvedIdx->variableType != IntClass) {
			gfx_printf("[FATAL] index is not an integer");
			return NULL;
		}
			

		return callMemberFunctionDirect(var, "get", &solvedIdx);
	}

	return NULL;
}

Variable_t* genericCallDirect(Variable_t* var, Variable_t** args, u8 len) {
	if (var->variableType != FunctionClass)
		return NULL;

	if (var->function.builtIn) {
		for (u32 i = 0; i < var->function.len; i++) {
			if (var->function.builtInPtr[i].argCount == len || var->function.builtInPtr[i].argCount == VARARGCOUNT) {
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

	gfx_printf("[FATAL] Arguments do not match function defenition");
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
			//Vector_t argsHolder = newVec(sizeof(Variable_t*), 1);
			Variable_t** argsHolder = NULL;
			if (var->function.builtInPtr->argCount != 0)
				argsHolder = malloc(sizeof(Variable_t*) * var->function.builtInPtr->argCount);
			int argCount = 0;
			int lasti = 0;
			Operator_t* ops = f->operations.data;

			int tooManyArgs = 0;

			// Loops trough the function to get all args out
			for (int i = 0; i < f->operations.count; i++) {
				if (ops[i].token == EquationSeperator || i + 1 == f->operations.count) {
					if (i + 1 == f->operations.count)
						i++;

					if (argCount == var->function.builtInPtr->argCount) {
						tooManyArgs = 1;
						break;
					}

					if (var->function.firstArgAsFunction && argCount == 0) {
						Function_t f = { .operations = vecFromArray(&ops[lasti], i - lasti, sizeof(Operator_t)) };
						Variable_t var = newFunctionVariable(createFunctionClass(f, NULL));
						var.reference = 1;
						Variable_t* varPtr = copyVariableToPtr(var);
						//vecAdd(&argsHolder, varPtr);
						argsHolder[argCount++] = varPtr;
					}
					else {
						Variable_t* var = eval(&ops[lasti], i - lasti, 1);
						if (var == NULL)
							return NULL; // maybe free first?

						//vecAdd(&argsHolder, var);
						argsHolder[argCount++] = var;
					}

					lasti = i + 1;
				}
			}
			
			Variable_t* res = NULL;

			if (!tooManyArgs)
				res = genericCallDirect(var, argsHolder, argCount);
			else
				gfx_printf("[FATAL] too many args provided");

			for (int i = 0; i < argCount; i++)
				removePendingReference(argsHolder[i]);

			//vecForEach(Variable_t**, tofree, (&argsHolder))
			//	removePendingReference(*tofree);

			FREE(argsHolder);

			return res;
		}
	}
	else {
		eval(var->function.function.operations.data, var->function.function.operations.count, 0);
		return &emptyClass;
	}
}

Variable_t getGenericFunctionMember(Variable_t* var, char* memberName, ClassFunctionTableEntry_t* entries, u8 len) {
	Variable_t newVar = {.readOnly = 1, .variableType = FunctionClass};
	newVar.function.origin = var;
	newVar.function.builtIn = 1;
	for (u32 i = 0; i < len; i++) {
		if (!strcmp(entries[i].name, memberName)) {
			newVar.function.builtInPtr = &entries[i];
			
			u32 j = i;
			for (; j < len && !strcmp(entries[j].name, memberName); j++);
			newVar.function.len = j - i;

			return newVar;
		}
	}

	gfx_printf("[FATAL] Did not find member with specified name");
	return (Variable_t){ 0 };
}

Variable_t* callMemberFunction(Variable_t* var, char* memberName, CallArgs_t* args) {
	for (u32 i = 0; i < ARRAY_SIZE(memberGetters); i++) {
		if (var->variableType == memberGetters[i].classType) {
			Variable_t funcRef = memberGetters[i].func(var, memberName);
			if (funcRef.variableType == None)
				return NULL;

			return genericCall(&funcRef, args);
		}
	}

	return NULL;
}

Variable_t* callMemberFunctionDirect(Variable_t* var, char* memberName, Variable_t** other) {
	for (u32 i = 0; i < ARRAY_SIZE(memberGetters); i++) {
		if (var->variableType == memberGetters[i].classType) {
			Variable_t funcRef = memberGetters[i].func(var, memberName);
			if (funcRef.variableType == None)
				return NULL;

			Variable_t* callRes = genericCallDirect(&funcRef, other, (other == NULL) ? 0 : 1);
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