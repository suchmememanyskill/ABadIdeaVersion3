#include "unsolvedArrayClass.h"
#include "eval.h"
#include "compat.h"
#include "intClass.h"

Variable_t* getStaticVariableFromOp(Operator_t* op, Variable_t *staticVars) {
	Variable_t* var = NULL;
	if (op->variable.staticVariableSet) {
		if (op->variable.staticVariableRef) {
			op->variable.staticVariable = &staticVars[(int)op->variable.staticVariable];
			op->variable.staticVariableRef = 0;
		}

		var = op->variable.staticVariable;
		var->readOnly = 1;
		var->reference = 1;
		var->gcDoNotFree = 1;
	}

	return var;
}

Variable_t* solveArray(Variable_t *unsolvedArray) {
	if (unsolvedArray->unsolvedArray.operations.count <= 0) {
		//Variable_t 
		// Return empty unsolved array that turns into a solved array once something is put into it
	}
}

Variable_t createUnsolvedArrayVariable(Function_t* f, Vector_t* staticVars) {
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

		Variable_t* staticVarPtr = staticVars->data;
		for (int i = staticVars->count - holder.count; i < staticVars->count; i++) {
			Variable_t* currentVarPtr = &staticVarPtr[i];
			//freeVariable(&currentVarPtr);
			// TODO: free inside variable!
		}

		//staticVars->count -= holder.count;

		vecFree(f->operations); // TODO: clean properly
		var.solvedArray.vector = holder;
		var.readOnly = 1;
	}
	else {
		var.unsolvedArray.operations = f->operations;
		var.variableType = UnresolvedArrayClass;
	}


	return var;
}