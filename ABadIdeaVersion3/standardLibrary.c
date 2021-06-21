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

	if (value)
		return genericCallDirect(args[1], NULL, 0);

	return &emptyClass;
}

// TODO: implement else by making if return a class that is else-able

enum standardFunctionIndexes {
	STD_IF = 0,
};

u8 oneIntoneFunction[] = { IntClass, FunctionClass };

ClassFunctionTableEntry_t standardFunctionDefenitions[] = {
	[STD_IF] = {"if", stdIf, 2, oneIntoneFunction},
};

#define createStandardFunction(classFunctionTableEntry) {.variableType = FunctionClass, .readOnly = 1, .gcDoNotFree = 1, .function.builtIn = 1, .function.builtInPtr = &classFunctionTableEntry, .function.len = 1}

Variable_t standardFunctions[] = {
	[STD_IF] = createStandardFunction(standardFunctionDefenitions[STD_IF]),
};

Dict_t standardLibrary[] = {
	[STD_IF] = {.name = "if", .var = &standardFunctions[STD_IF]},
};

Variable_t* searchStdLib(char* funcName) {
	for (int i = 0; i < ARRAY_SIZE(standardLibrary); i++) {
		if (!strcmp(funcName, standardLibrary[i].name))
			return standardLibrary[i].var;
	}

	return NULL;
}