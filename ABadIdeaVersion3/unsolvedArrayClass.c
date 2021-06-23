#include "unsolvedArrayClass.h"
#include "eval.h"
#include "compat.h"
#include "intClass.h"
#include <string.h>

Variable_t* solveArray(Variable_t *unsolvedArray) {
	if (unsolvedArray->unsolvedArray.operations.count <= 0) {
		return unsolvedArray;
		// Return empty unsolved array that turns into a solved array once something is put into it
	}

	vecForEach(Operator_t*, curOp, (&unsolvedArray->unsolvedArray.operations)) {

	}
}

Variable_t createUnsolvedArrayVariable(Function_t* f) {
	Variable_t var = { 0 };
	Vector_t holder = { 0 };
	u8 varType = Invalid;
	
	// Foreach to attempt to create the array. Should fail if calcs are done or types are not equal
	if (f->operations.count > 0) {
		vecForEach(Operator_t*, curOp, (&f->operations)) {
			if (holder.data == NULL) {
				if (curOp->variable.staticVariableType == 1) {
					varType = IntClass;
					holder = newVec(sizeof(s64), 4);
					vecAdd(&holder, (curOp->variable.integerType));
				}
				else if (curOp->variable.staticVariableType == 2) {
					if (!strcmp(curOp->variable.stringType, "BYTE[]")) {
						varType = ByteArrayClass; // Repurpose varType
						holder = newVec(sizeof(u8), 4);
						FREE(curOp->variable.stringType);
					}
					else {
						varType = StringClass;
						holder = newVec(sizeof(char*), 4);
						vecAdd(&holder, (curOp->variable.stringType));
					}
				}
				else {
					break;
				}
			}
			else {
				if ((curOp - 1)->token == EquationSeperator && curOp->token == Variable) {
					if (curOp->variable.staticVariableType == 1) {
						if (varType == IntClass) {
							vecAdd(&holder, curOp->variable.integerType);
						}
						else if (varType == ByteArrayClass) {
							u8 var = (u8)(curOp->variable.integerType & 0xFF);
							vecAdd(&holder, var);
						}
					}
					else if (curOp->variable.staticVariableType == 2) {
						if (varType == StringClass) {
							vecAdd(&holder, curOp->variable.stringType);
						}
					}
					else {
						vecFree(holder);
						holder.data = NULL;
						break;
					}
				}
				else if (curOp->token == EquationSeperator) {
					continue;
				}
				else {
					vecFree(holder);
					holder.data = NULL;
					break;
				}
			}
		}
	}

	if (varType != Invalid) {
		if (varType == IntClass) {
			var.variableType = IntArrayClass;
		}
		else if (varType == StringClass) {
			var.variableType = StringArrayClass;
		}
		else {
			var.variableType = varType;
		}

		vecFree(f->operations);
		var.solvedArray.vector = holder;
		var.readOnly = 1;
	}
	else {
		var.unsolvedArray.operations = f->operations;
		var.variableType = UnresolvedArrayClass;
	}


	return var;
}