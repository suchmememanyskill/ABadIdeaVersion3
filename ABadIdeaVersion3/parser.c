#include "model.h"
#include "compat.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

#include "intClass.h"
#include "StringClass.h"
#include "unsolvedArrayClass.h"
#include "functionClass.h"

static inline int isValidWord(char c) {
	char r = c | 0x20;
	return ((r >= 'a' && r <= 'z') || c == '_');
}

static inline int isValidNum(char c) {
	return (c >= '0' && c <= '9');
}

static inline int isValidVar(char c) {
	return (isValidWord(c) || isValidNum(c));
}

static inline int isValidHexNum(char c) {
	char r = c | 0x20;
	return (isValidNum(r) || (r >= 'a' && r <= 'f'));
}

char* getTokenText(u8 token) {
	for (u32 i = 0; i < tokenConvertionCount; i++) {
		if (tokenConvertions[i].token == token)
			return tokenConvertions[i].strToken;
	}

	return NULL;
}

char* utils_copyStringSize(const char* in, int size) {
	if (size > strlen(in) || size < 0)
		size = strlen(in);

	char* out = calloc(size + 1, 1);
	//strncpy(out, in, size);
	if (size)
		memcpy(out, in, size);
	return out;
}

#define ELIFC(c) else if (*in == c)

Vector_t script;

enum TokenType {
	Token_Variable = 0,
	Token_String,
	Token_Int,
	Token_Token
};

typedef enum {
	History_Function = 0,
	History_Bracket,
	History_Array,
} StackHistory_t;

Vector_t functionStack; // Function_t
Vector_t bracketStack; // Operator_t
UnsolvedArrayClass_t arrayHolder; // No recursive array elements are currently supported
Vector_t stackHistoryHolder; // StackHistory_t
Vector_t staticVariableHolder; // Variable_t. Malloc should not be ran once per static variable. Like this we only have to malloc once

Function_t* StartParse(char* in) {
	// TODO: change later to Equation_t
	script = newVec(sizeof(Operator_t), 16);

	functionStack = newVec(sizeof(Function_t), 4);
	Function_t firstFunction = createEmptyFunction();
	vecAdd(&functionStack, firstFunction);

	staticVariableHolder = newVec(sizeof(Variable_t), 16);

	stackHistoryHolder = newVec(sizeof(StackHistory_t), 4);
	StackHistory_t firstHistory = History_Function;
	vecAdd(&stackHistoryHolder, firstHistory);

	while (*in) {
		if (*in == '#') {
			// add flag #REQUIRE MINERVA and #REQUIRE VER 3.0.6 later

			if (!memcmp(in + 1, "REQUIRE ", 8)) {
				if (!memcmp(in + 9, "VER ", 4)) {
					u8 vers[3] = { 0 };
					char* verStart = in + 13;
					for (u8 i = 0; i < 3; i++) {
						while (isValidNum(*verStart)) {
							vers[i] = vers[i] * 10 + *verStart++ - '0';
						}
						verStart++;
					}

					u8 outdated = 0;
					if (vers[0] > LPVERSION_MAJOR)
						outdated = 1;
					else if (vers[0] == LPVERSION_MAJOR) {
						if (vers[1] > LPVERSION_MINOR)
							outdated = 1;
						else if (vers[1] == LPVERSION_MINOR) {
							if (vers[2] > LPVERSION_BUGFX)
								outdated = 1;
						}
					}

					if (outdated)
						gfx_printf("[FATAL] Script runner is outdated!");
				}
				else if (!memcmp(in + 9, "MINERVA", 7)) {
					u8 minervaEnabled = 0; // TODO: Change this to the actual value
					if (!minervaEnabled)
						gfx_printf("[FATAL] extended memory required");
				}
			}

			while (*in && *in != '\n')
				in++;
		}
		else if (isValidWord(*in)) {
			char* startWord = in;
			in++;
			while (isValidVar(*in))
				in++;

			char* fuck = utils_copyStringSize(startWord, in - startWord);

			gfx_printf("Variable: '%s'\n", fuck);
			NextTokenv2(Token_Variable, fuck);

			continue;
		}
		else if (isValidNum(*in) || (*in == '-' && isValidNum(in[1]))) {
			s64 parse = 0;
			u8 negative = (*in == '-');
			if (negative)
				in++;

			if (*in == '0' && (in[1] | 0x20) == 'x') {
				in += 2;
				while (isValidHexNum(*in)) {
					parse = parse * 16 + (*in & 0x0F) + (*in >= 'A' ? 9 : 0);
					in++;
				}
			}
			else while (isValidNum(*in)) {
				parse = parse * 10 + *in++ - '0';
			}

			if (negative)
				parse *= -1;

			gfx_printf("Integer: '%d'\n", parse);
			NextTokenv2(Token_Int, &parse);

			continue;
		}
		ELIFC('"') {
			char* startStr = ++in;
			int len = 0;
			while (*in != '"') {
				in++;
			}
			len = in - startStr;

			char* storage = malloc(len + 1);

			int pos = 0;
			for (int i = 0; i < len; i++) {
				if (startStr[i] == '\\') {
					if (startStr[i + 1] == 'n') {
						storage[pos++] = '\n';
						i++;
						continue;
					}

					if (startStr[i + 1] == 'r') {
						storage[pos++] = '\r';
						i++;
						continue;
					}
				}
				storage[pos++] = startStr[i];
			}
			storage[pos] = '\0';

			gfx_printf("String: '%s'\n", storage);
			NextTokenv2(Token_String, storage);
		}
		else {
			for (u32 i = 0; i < tokenConvertionCount; i++) {
				TokenConvertion_t t = tokenConvertions[i];
				if (!memcmp(t.strToken, in, (t.strToken[1] == '\0') ? 1 : 2)) {
					gfx_printf("Token: '%s'\n", t.strToken);
					u8 tokenStorage = t.token;
					NextTokenv2(Token_Token, &tokenStorage);

					if (t.strToken[1] != '\0')
						in++;

					break;
				}
			}
		}
		in++;
	}

	return functionStack.data;
}

#define CreateVariableReferenceStatic(var) VariableReference_t reference = { .staticVariable = var, .action = ActionGet, .staticVariableSet = 1, .staticVariableRef = 1 }
#define CreateVariableReferenceStr(str) VariableReference_t reference = { .name = str, .action = ActionGet }

// TODO: all static variables should go into an array, so there are not 1000 mallocs!!

int NextTokenv2(u8 TokenType, void* item) {
	Function_t* lastFunc = getStackEntry(&functionStack);
	Operator_t* lastOp = NULL;
	if (lastFunc) {
		lastOp = getStackEntry(&lastFunc->operations);
	}

	Operator_t* lastBracket = getStackEntry(&bracketStack);
	StackHistory_t* lastHistory = getStackEntry(&stackHistoryHolder);

	Operator_t op = { .token = Variable };
	
	if (TokenType >= Token_Variable && TokenType <= Token_Int && lastOp) {
		if ((lastOp->token == Variable || lastOp->token == BetweenBrackets) && lastOp->variable.action != ActionSet) {
			op.token = EquationSeperator;
			vecAdd(&lastFunc->operations, op);
			op.token = Variable;
		}
	}

	if (TokenType == Token_Variable) {
		CreateVariableReferenceStr(item);
		op.variable = reference;
	}
	else if (TokenType == Token_Int) {
		Variable_t a = newIntVariable(*((int*)item), 1);
		vecAdd(&staticVariableHolder, a);
		CreateVariableReferenceStatic((Variable_t*)(staticVariableHolder.count - 1));
		op.variable = reference;
	}
	else if (TokenType == Token_String) {
		Variable_t a = newStringVariable(item, 1, 0);
		vecAdd(&staticVariableHolder, a);
		CreateVariableReferenceStatic((Variable_t*)(staticVariableHolder.count - 1));
		op.variable = reference;
	}
	else if (TokenType == Token_Token) {
		u8 token = *(u8*)item;

		if (token == Equals && lastOp) {
			if (lastOp->token == Variable) {
				if (lastOp->variable.staticVariableSet) {
					gfx_printf("[FATAL] Trying to assign to a static variable");
				}
				else {
					lastOp->variable.action = ActionSet;
					return;
				}
			}
			else {
				gfx_printf("[FATAL] Trying to assign to non-object");
			}
		}
		else if (token == LeftCurlyBracket) {
			Function_t templateFunction = createEmptyFunction();
			vecAdd(&functionStack, templateFunction);

			StackHistory_t functionHistory = History_Function;
			vecAdd(&stackHistoryHolder, functionHistory);
			return;
		}
		else if (token == RightCurlyBracket) {
			if (stackHistoryHolder.count != 1 && *lastHistory == History_Function) {
				Variable_t a = newFunctionVariable(createFunctionClass(*lastFunc, NULL));
				popStackEntry(&functionStack);
				popStackEntry(&stackHistoryHolder);

				lastFunc = getStackEntry(&functionStack);

				vecAdd(&staticVariableHolder, a);
				CreateVariableReferenceStatic((Variable_t*)(staticVariableHolder.count - 1));
				op.variable = reference;
			}
			else {
				gfx_printf("[FATAL] stack count is 1 or state is not a function");
			}
		}
		else {
			op.token = token;
		}
	}

	if (*lastHistory == History_Function) {
		vecAdd(&lastFunc->operations, op);
	}
	else if (*lastHistory == History_Array) {
		// stub
	}
	else if (*lastHistory == History_Bracket) {
		// stub
	}

	return 0;
}

/*
int NextToken(u8 TokenType, void *item) {
	Operator_t* operators = vecGetArray(script, Operator_t*);
	Operator_t* lastOp = NULL;
	if (script.count > 0) {
		lastOp = &operators[script.count - 1];
	}

	Operator_t a = { 0 };

	if (TokenType == Token_Variable) {
		a.token = Variable;
		VariableReference_t b = { 0 };
		b.action = ActionGet;
		b.name = item;
		a.variable = b;
	}
	else if (TokenType == Token_Int) {
		a.token = Variable;
		VariableReference_t b = { 0 };
		b.action = ActionGet;
		b.staticVariableSet = 1;
		b.staticVariable = newIntVariablePtr(*((int*)item), 1);
		a.variable = b;
	}
	else if (TokenType == Token_String) {
		a.token = Variable;
		VariableReference_t b = { 0 };
		b.action = ActionGet;
		b.staticVariableSet = 1;
		b.staticVariable = newStringVariablePtr(item, 1, 0);
		a.variable = b;
	}
	else if (TokenType == Token_Token) {
		u8 token = *(u8*)item;
		if (token == Equals) {
			if (lastOp->token == Variable) {
				if (!lastOp->variable.staticVariableSet)
					lastOp->variable.action = ActionSet;
			}
			else 
				gfx_printf("[FATAL] cannot assign equals");

			return;
		}
		else {
			a.token = token;
		}
	}

	vecAdd(&script, a);
}
*/