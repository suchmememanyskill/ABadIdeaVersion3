#include "model.h"
#include "compat.h"
#include "genericClass.h"
#include "eval.h"
#include "garbageCollector.h"

Variable_t* staticVars;

void setStaticVars(Vector_t* vec) {
	staticVars = vec->data;
}

Variable_t* opToVar(Operator_t* op) {
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
			}

			var = op->variable.staticVariable;
			var->readOnly = 1;
			var->reference = 1;
			var->gcDoNotFree = 1;
		}
		else {
			// Stubbed
		}
	}

	while (args) {
		Variable_t* varNext = NULL;
		if (args->action == ActionGet) {
			varNext = genericGet(var, args);
		}
		else if (args->action == ActionSet) {
			gfx_printf("[FATAL] Unexpected set!");
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

Variable_t* eval(Operator_t* ops, u32 len, u8 ret) {
	Variable_t* set = NULL;
	Variable_t* curRes = NULL;
	Operator_t* curOp = NULL;
	for (u32 i = 0; i < len; i++) {
		Operator_t* cur = &ops[i];

		if (cur->token == CallArgs)
			continue;

		if (cur->token == EquationSeperator) {
			removePendingReference(curRes);

			curRes = NULL;
			curOp = NULL;
			continue;
		}

		if (curRes == NULL) {
			if (cur->token != Variable && cur->token != BetweenBrackets)
				gfx_printf("[FATAL] First token is not a variable");
			else {
				curRes = opToVar(cur);
				if (!curRes) {
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

		Variable_t* rightSide = opToVar(cur);
		if (!rightSide) {
			gfx_printf("[FATAL] Invalid variable operator");
		}

		Variable_t* result = callMemberFunctionDirect(curRes, curOp->tokenStr, &rightSide);
		// Free old values

		removePendingReference(curRes);
		removePendingReference(rightSide);
		rightSide = NULL;

		curRes = result;
	}

	if (!ret) {
		removePendingReference(curRes);
		return NULL;
	}

	return curRes;
}