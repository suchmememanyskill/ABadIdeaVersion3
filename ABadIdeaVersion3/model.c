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

Variable_t emptyClass = { .variableType = EmptyClass, .readOnly = 1, .reference = 1, .gcDoNotFree = 1 };