#include "genericClass.h"
#include "model.h"
#include "intClass.h"
#include "compat.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>

Variable_t* copyVariableToPtr(Variable_t var) {
	Variable_t* a = malloc(sizeof(Variable_t));
	*a = var;
	return a;
}

MemberGetters_t memberGetters[] = {
	{IntClass, getIntegerMember},
};

Variable_t* genericGet(Variable_t* var, VariableReference_t* ref) {
	if (ref->extraAction == ActionExtraMemberName) {
		for (u32 i = 0; i < ARRAY_SIZE(memberGetters); i++) {
			if (var->variableType == memberGetters[i].classType)
				return memberGetters[i].func(var, ref->extra);
		}
	}

	return NULL;
}

Variable_t* genericCall(Variable_t* var, VariableReference_t* ref) {
	// Stubbed
}

Variable_t* genericCallDirect(Variable_t* var, Variable_t** args, u8 len) {
	if (var->variableType != FunctionClass)
		return NULL;

	if (var->function.builtIn) {
		for (u32 i = 0; i < var->function.len; i++) {
			if (var->function.builtInPtr[i].argCount == len) {
				int valid = 1;
				if (var->function.len != 0) {
					for (u32 j = 0; j < var->function.builtInPtr[i].argCount; j++) {
						if (var->function.builtInPtr[i].argTypes[j] != args[j]->variableType) {
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
		// Stubbed
	}

	return NULL;
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

			return copyVariableToPtr(newVar);
		}
	}

	return NULL;
}