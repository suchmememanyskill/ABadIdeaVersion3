#include "model.h"
#include "compat.h"
#include "vector.h"
#include "parser.h"
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
	Token_Token,
	Token_Err,
};

typedef enum {
	History_Function = 0,
	History_Bracket,
	History_Array,
} StackHistory_t;
/*
Vector_t functionStack; // Function_t
Vector_t bracketStack; // Operator_t
UnsolvedArrayClass_t arrayHolder; // No recursive array elements are currently supported
Vector_t stackHistoryHolder; // StackHistory_t
Vector_t staticVariableHolder; // Variable_t. Malloc should not be ran once per static variable. Like this we only have to malloc once
*/

u8 nextToken(char** inPtr, void** val) {
	char* in = *inPtr;
	u8 ret = Token_Err;
	while (ret == Token_Err) {
		if (*in == '#') {
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

			char* str = utils_copyStringSize(startWord, in - startWord);

			gfx_printf("Variable: '%s'\n", str);
			ret = Token_Variable;
			*val = str;
			break;
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
			ret = Token_Int;

			s64* parsePersistent = malloc(sizeof(s64));
			*parsePersistent = parse;

			*val = parsePersistent;
			break;
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
			ret = Token_String;
			*val = storage;
		}
		else {
			for (u32 i = 0; i < tokenConvertionCount; i++) {
				TokenConvertion_t t = tokenConvertions[i];
				if (!memcmp(t.strToken, in, (t.strToken[1] == '\0') ? 1 : 2)) {
					gfx_printf("Token: '%s'\n", t.strToken);
					ret = Token_Token;
					*val = t.token;

					if (t.strToken[1] != '\0')
						in++;

					break;
				}
			}
		}
		in++;
	}

	*inPtr = in;
	return ret;
}

#define CreateVariableReferenceStatic(var) VariableReference_t reference = { .staticVariable = var, .action = ActionGet, .staticVariableSet = 1, .staticVariableRef = 1 }
#define CreateVariableReferenceStr(str) VariableReference_t reference = { .name = str, .action = ActionGet }

void setLastActionVariableRef(VariableReference_t* ref, ActionType_t action) {
	for (; ref->subcall != NULL; ref = ref->subcall);
	ref->action = action;
}

void addExtraOnVariableRef(VariableReference_t *ref, ActionExtraType_t actionExtra, void *extra) {
	for (; ref->subcall != NULL; ref = ref->subcall);
	// Check for FuncCall -> FuncCallArgs?
	if (ref->extraAction == ActionExtraNone) {
		ref->extraAction = actionExtra;
		ref->extra = extra;
	}
	else {
		VariableReference_t* newRef = calloc(1, sizeof(VariableReference_t));
		newRef->extraAction = actionExtra;
		newRef->extra = extra;
		ref->subcall = newRef;
	}
}

ParserRet_t parseScript(char* in) {
	Vector_t functionStack; // Function_t
	UnsolvedArrayClass_t arrayHolder;
	Vector_t stackHistoryHolder; // StaticHistory_t
	Vector_t staticVariableHolder; // Variable_t

	functionStack = newVec(sizeof(Function_t), 4);
	Function_t firstFunction = createEmptyFunction();
	vecAdd(&functionStack, firstFunction);

	staticVariableHolder = newVec(sizeof(Variable_t), 16);

	stackHistoryHolder = newVec(sizeof(StackHistory_t), 4);
	StackHistory_t firstHistory = History_Function;
	vecAdd(&stackHistoryHolder, firstHistory);

	while (*in) {
		Function_t* lastFunc = getStackEntry(&functionStack);
		StackHistory_t* lastHistory = getStackEntry(&stackHistoryHolder);

		Operator_t* lastOp = NULL;

		if (*lastHistory == History_Bracket || *lastHistory == History_Function) {
			if (lastFunc) {
				lastOp = getStackEntry(&lastFunc->operations);
			}
		}
		else if (*lastHistory == History_Array) {
			lastOp = getStackEntry(&arrayHolder.operations);
		}

		void* var = NULL;
		u8 tokenType = nextToken(&in, &var);

		Operator_t op = { .token = Variable };

		if (tokenType >= Token_Variable && tokenType <= Token_Int && lastOp) {
			if ((lastOp->token == Variable || lastOp->token == BetweenBrackets) && lastOp->variable.action != ActionSet) {
				op.token = EquationSeperator;
				if (*lastHistory == History_Function || *lastHistory == History_Bracket) {
					vecAdd(&lastFunc->operations, op);
				}
				else if (*lastHistory == History_Array) {
					vecAdd(&arrayHolder.operations, op);
				}
				op.token = Variable;
			}
		}

		if (tokenType == Token_Variable) {
			CreateVariableReferenceStr(var);
			op.variable = reference;
		}
		else if (tokenType == Token_Int) {
			Variable_t a = newIntVariable(*((s64*)var), 1);
			free(var);
			vecAdd(&staticVariableHolder, a);
			CreateVariableReferenceStatic((Variable_t*)(staticVariableHolder.count - 1));
			op.variable = reference;
		}
		else if (tokenType == Token_String) {
			Variable_t a = newStringVariable(var, 1, 0);
			vecAdd(&staticVariableHolder, a);
			CreateVariableReferenceStatic((Variable_t*)(staticVariableHolder.count - 1));
			op.variable = reference;
		}
		else if (tokenType == Token_Token) {
			u8 token = (u8)var;

			if (token == Equals && lastOp) {
				if (lastOp->token == Variable) {
					if (lastOp->variable.staticVariableSet) {
						gfx_printf("[FATAL] Trying to assign to a static variable");
					}
					else {
						lastOp->variable.action = ActionSet;
						continue;
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
				continue;
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

					// TODO: detect if lastFunc is a variable with Call
				}
				else {
					gfx_printf("[FATAL] stack count is 1 or state is not a function");
				}
			}
			else if (token == Dot) {
				if (lastOp->token != Variable)
					gfx_printf("[FATAL] member access on non-variable");
				else {
					tokenType = nextToken(&in, &var);
					if (tokenType != Token_Variable) {
						gfx_printf("[FATAL] acessing member with non-dynamic token");
					}
					else {
						addExtraOnVariableRef(&lastOp->variable, ActionExtraMemberName, var);
						continue;
					}
				}
			}
			else if (token == LeftBracket) {
				Function_t templateFunction = createEmptyFunction();
				vecAdd(&functionStack, templateFunction);

				StackHistory_t functionHistory = History_Bracket;
				vecAdd(&stackHistoryHolder, functionHistory);
				continue;
			}
			else if (token == RightBracket) {
				if (*lastHistory == History_Bracket) {
					Function_t* bstack = popStackEntry(&functionStack);
					popStackEntry(&stackHistoryHolder);
					lastFunc = getStackEntry(&functionStack);
					lastHistory = getStackEntry(&stackHistoryHolder);

					// Copy paste
					if (*lastHistory == History_Bracket || *lastHistory == History_Function) {
						if (lastFunc) {
							lastOp = getStackEntry(&lastFunc->operations);
						}
					}
					else if (*lastHistory == History_Array) {
						lastOp = getStackEntry(&arrayHolder.operations);
					}

					if (lastOp->token == Variable) {
						Function_t* newBStack = malloc(sizeof(Function_t));
						*newBStack = *bstack;
						addExtraOnVariableRef(&lastOp->variable, ActionExtraCallArgs, newBStack);
						continue;
					}
					else {
						op.betweenBrackets = *bstack;
						op.token = BetweenBrackets;
					}
				}
				else {
					gfx_printf("[FATAL] ) without (");
				}
			}
			else {
				op.token = token;
			}
		}

		if (*lastHistory == History_Function || *lastHistory == History_Bracket) {
			vecAdd(&lastFunc->operations, op);
		}
		else if (*lastHistory == History_Array) {
			// stub
		}
	}

	if (functionStack.count != 1 || stackHistoryHolder.count != 1)
		gfx_printf("[FATAL] there seems to be an open bracket somewhere. EOF reached");

	ParserRet_t parse = { .main = (*(Function_t*)getStackEntry(&functionStack)), .staticVarHolder = staticVariableHolder };

	vecFree(functionStack);
	vecFree(stackHistoryHolder);

	return parse;
}