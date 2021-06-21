#include "model.h"
#include "compat.h"
#include "genericClass.h"
#include "eval.h"
#include "garbageCollector.h"
#include "vector.h"
#include "standardLibrary.h"
#include <string.h>

Variable_t* staticVars;

void setStaticVars(Vector_t* vec) {
	staticVars = vec->data;
}

Vector_t runtimeVars;

void initRuntimeVars() {
	runtimeVars = newVec(sizeof(Dict_t), 8);
}

void exitRuntimeVars() {
	
	vecForEach(Dict_t*, variableArrayEntry, (&runtimeVars)) {
		removePendingReference(variableArrayEntry->var);
	}

	processPendingReferences();

	vecFree(runtimeVars);	
}

Variable_t* opToVar(Operator_t* op, Callback_SetVar_t *setCallback) {
	Variable_t* var = NULL;
	CallArgs_t* args = NULL;

	if ((op + 1)->token == CallArgs)
		args = &(op + 1)->callArgs;

	if (op->token == BetweenBrackets) {
		var = eval(op->variable.betweenBrackets.data, op->variable.betweenBrackets.len, 1);
	}
	else if (op->token == Variable) {
		if (op->variable.staticVariableSet) {
			if (op->variable.staticVariableRef) {
				op->variable.staticVariable = &staticVars[(int)op->variable.staticVariable];
				op->variable.staticVariableRef = 0;
				op->variable.staticVariable->readOnly = 1;
				op->variable.staticVariable->reference = 1;
				op->variable.staticVariable->gcDoNotFree = 1;
			}

			var = op->variable.staticVariable;
		}
		else {
			var = searchStdLib(op->variable.name);

			if (var == NULL) {
				if (args != NULL) {
					if (args->action == ActionSet) {
						setCallback->isTopLevel = 1;
						setCallback->varName = op->variable.name;
						setCallback->hasVarName = 1;
						return NULL;
					}
				}

				vecForEach(Dict_t*, variableArrayEntry, (&runtimeVars)) {
					if (!strcmp(variableArrayEntry->name, op->variable.name)) {
						var = variableArrayEntry->var;
						break;
					}
				}
			}

			if (var == NULL)
				return NULL;

			addPendingReference(var);
		}
	}

	while (args) {
		Variable_t* varNext = NULL;
		if (args->action == ActionGet) {
			varNext = genericGet(var, args);
		}
		else if (args->action == ActionSet) {
			if (args->extraAction == ActionExtraMemberName || args->extraAction == ActionExtraArrayIndex) {
				setCallback->hasVarName = (args->extraAction == ActionExtraMemberName) ? 1 : 0;
				setCallback->setVar = var;
				if (args->extraAction == ActionExtraMemberName) {
					setCallback->varName = args->extra;
				}
				else {
					setCallback->idxVar = args->extra;
				}
			}
			else {
				gfx_printf("[FATAL] Unexpected set!");
			}
			return NULL;
		}
		else if (args->action == ActionCall) {
			varNext = genericCall(var, args);
		}

		if (varNext == NULL)
			return NULL;

		removePendingReference(var);

		//if (!var->reference)
		//	freeVariable(&var);

		var = varNext;
		args = args->next;
	}

	// Stubbed
	return var;
}

void runtimeVariableEdit(Callback_SetVar_t* set, Variable_t* curRes) {
	if (set->isTopLevel) {
		vecForEach(Dict_t*, variableArrayEntry, (&runtimeVars)) {
			if (!strcmp(variableArrayEntry->name, set->varName)) {
				removePendingReference(variableArrayEntry->var);
				//addPendingReference(curRes);
				variableArrayEntry->var = curRes;
				return;
			}
		}

		Dict_t newStoredVariable = { 0 };
		newStoredVariable.name = CpyStr(set->varName);
		newStoredVariable.var = curRes;
		vecAdd(&runtimeVars, newStoredVariable);
		return;
	}

	// TODO: add non-top level sets
}

Variable_t* eval(Operator_t* ops, u32 len, u8 ret) {
	Variable_t* curRes = NULL;
	Operator_t* curOp = NULL;
	Callback_SetVar_t set = { 0 };
	for (u32 i = 0; i < len; i++) {
		Operator_t* cur = &ops[i];

		if (cur->token == CallArgs)
			continue;

		if (cur->token == EquationSeperator) {
			if (set.hasBeenNoticed == 1) 
				runtimeVariableEdit(&set, curRes);
			else
				removePendingReference(curRes);

			memset(&set, 0, sizeof(Callback_SetVar_t));
			curRes = NULL;
			curOp = NULL;
			continue;
		}

		if (curRes == NULL) {
			if (cur->token != Variable && cur->token != BetweenBrackets)
				gfx_printf("[FATAL] First token is not a variable");
			else {
				curRes = opToVar(cur, &set);
				if (!curRes) {
					if ((set.varName != NULL || set.idxVar != NULL) && set.hasBeenNoticed == 0) {
						set.hasBeenNoticed = 1;
						continue;
					}
					gfx_printf("[FATAL] Invalid variable operator");
				}
			}
			continue;
		}

		if (curOp == NULL) {
			if (cur->token != Variable && cur->token != BetweenBrackets) {
				curOp = cur;
			}
			else {
				gfx_printf("[FATAL] First operator is not an operator");
			}
			continue;
		}

		Variable_t* rightSide = opToVar(cur, &set);
		if (!rightSide) {
			gfx_printf("[FATAL] Invalid variable operator");
		}

		// Issue lies here for freeing issues, curRes is corrupted
		Variable_t* result = callMemberFunctionDirect(curRes, curOp->tokenStr, &rightSide);
		// Free old values

		removePendingReference(curRes);
		removePendingReference(rightSide);
		rightSide = NULL;
		curOp = NULL;

		curRes = result;
	}

	if (set.hasBeenNoticed == 1) {
		runtimeVariableEdit(&set, curRes);
		return &emptyClass;
	}
	else if (!ret) {
		removePendingReference(curRes);
		return &emptyClass;
	}

	return curRes;
}