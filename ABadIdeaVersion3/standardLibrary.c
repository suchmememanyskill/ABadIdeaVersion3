#include "model.h"
#include "compat.h"
#include "genericClass.h"
#include "eval.h"
#include "garbageCollector.h"
#include "vector.h"
#include "intClass.h"
#include "standardLibrary.h"
#include <string.h>

ClassFunction(stdIf) {
	s64 value = getIntValue(args[0]);


	if (value) {
		Variable_t* res = genericCallDirect(args[1], NULL, 0);
		if (res == NULL)
			return NULL;

		removePendingReference(res);
	}

	return &emptyClass;
}

// TODO: implement else by making if return a class that is else-able

ClassFunction(stdWhile) {
	Variable_t* result = eval(args[0]->function.function.operations.data, args[0]->function.function.operations.count, 1);
	if (result == NULL || result->variableType != IntClass)
		return NULL;

	removePendingReference(result);

	while (result->integer.value) {
		Variable_t* res = genericCallDirect(args[1], NULL, 0);
		if (res == NULL)
			return NULL;

		removePendingReference(res);

		result = eval(args[0]->function.function.operations.data, args[0]->function.function.operations.count, 1);
		if (result == NULL || result->variableType != IntClass)
			return NULL;

		removePendingReference(result);
	}

	return &emptyClass;
}

ClassFunction(stdPrint) {
	
	for (int i = 0; i < argsLen; i++) {
		Variable_t* res = callMemberFunctionDirect(args[i], "print", NULL);
		if (res == NULL)
			return NULL;
	}
	

	return &emptyClass;
}

enum standardFunctionIndexes {
	STD_IF = 0,
	STD_WHILE,
	STD_PRINT,
};

u8 oneIntoneFunction[] = { IntClass, FunctionClass };
u8 doubleFunctionClass[] = { FunctionClass, FunctionClass };

ClassFunctionTableEntry_t standardFunctionDefenitions[] = {
	[STD_IF] = {"if", stdIf, 2, oneIntoneFunction},
	[STD_WHILE] = {"while", stdWhile, 2, doubleFunctionClass},
	[STD_PRINT] = {"print", stdPrint, VARARGCOUNT, 0},
};

#define createStandardFunction(classFunctionTableEntry) {.variableType = FunctionClass, .readOnly = 1, .gcDoNotFree = 1, .function.builtIn = 1, .function.builtInPtr = &classFunctionTableEntry, .function.len = 1}
#define createStandardFunctionWithFirstArgFunction(classFunctionTableEntry) {.variableType = FunctionClass, .readOnly = 1, .gcDoNotFree = 1, .function.builtIn = 1 , .function.firstArgAsFunction = 1, .function.builtInPtr = &classFunctionTableEntry, .function.len = 1}

Variable_t standardFunctions[] = {
	[STD_IF] = createStandardFunction(standardFunctionDefenitions[STD_IF]),
	[STD_WHILE] = createStandardFunctionWithFirstArgFunction(standardFunctionDefenitions[STD_WHILE]),
	[STD_PRINT] = createStandardFunction(standardFunctionDefenitions[STD_PRINT]),
};

Dict_t standardLibrary[] = {
	[STD_IF] = {.name = "if", .var = &standardFunctions[STD_IF]},
	[STD_WHILE] = {.name = "while", .var = &standardFunctions[STD_WHILE]},
	[STD_PRINT] = {.name = "print", .var = &standardFunctions[STD_PRINT]},
};

Variable_t* searchStdLib(char* funcName) {
	for (int i = 0; i < ARRAY_SIZE(standardLibrary); i++) {
		if (!strcmp(funcName, standardLibrary[i].name))
			return standardLibrary[i].var;
	}

	return NULL;
}