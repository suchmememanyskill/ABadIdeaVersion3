#include "unsolvedArrayClass.h"

Variable_t createUnsolvedArrayVariable(Function_t* f) {
	Variable_t var = { 0 };
	var.unsolvedArray.operations = f->operations;
	var.variableType = UnresolvedArrayClass;
	return var;
}