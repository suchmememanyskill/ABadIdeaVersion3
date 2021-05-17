#include "model.h"
#include "compat.h"
#include "StringClass.h"
#include "intClass.h"

TokenConvertion_t tokenConvertions[] = {
	{Not, "!"},

	{SmallerEqual, "<="},
	{BiggerEqual, "=>"},
	{NotEqual, "!="},
	{LogicAnd, "&&"},
	{LogicOr, "||"},

	{BitShiftLeft, "<<"},
	{BitShiftRight, ">>"},

	{Plus, "+"},
	{Equals, "="},
	{Minus, "-"},
	{Multiply, "*"},
	{Division, "/"},
	{Modulo, "%"},

	{LeftSquareBracket, "["},
	{LeftCurlyBracket, "{"},
	{LeftBracket, "("},
	{RightSquareBracket, "]"},
	{RightCurlyBracket, "}"},
	{RightBracket, ")"},

	{Smaller, "<"},
	{Bigger, ">"},
	
	{And, "&"},
	{Or, "|"},
	{Dot, "."},
	{EquationSeperator, ","},
};

u32 tokenConvertionCount = ARRAY_SIZE(tokenConvertions);

typedef struct {
	classFunctionTable classTable;
	u8 classType;
} ClassFunctionTableDefenitions_t;


ClassFunctionTableDefenitions_t defs[] = {
	{intFunctionHandler, IntClass},
	{stringFunctionHandler, StringClass},
};

classFunctionTable classTableSearch(u8 classType) {
	for (u32 i = 0; i < ARRAY_SIZE(defs); i++) {
		if (defs[i].classType == classType)
			return defs[i].classTable;
	}

	return NULL;
}

Variable_t* callClass(char* funcName, Variable_t* caller, VariableReference_t* ref, Vector_t* args) {
	if (caller) {
		classFunctionTable a = classTableSearch(caller->variableType);
		if (a) {
			return a(funcName, caller, ref, args);
		}
	}

	return NULL;
}
