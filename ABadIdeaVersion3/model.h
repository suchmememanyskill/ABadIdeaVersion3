#pragma once

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef long long s64;

typedef struct {
	void* data;
	u32 capacity;
	u32 count;
	u8 elemSz;
} Vector_t;

typedef enum {
	None = 0,
	IntClass,
	FunctionClass,
	StringClass,
	ArrayClass,
	UnresolvedArrayClass,
	DictionaryClass,
} VariableType_t;

typedef enum {
	Invalid = 0,
	Variable,
	BetweenBrackets,
	Not,

	Plus,
	Equals,
	Minus,
	Multiply,
	Division,
	Modulo,

	LeftSquareBracket,
	LeftCurlyBracket,
	LeftBracket,
	RightSquareBracket,
	RightCurlyBracket,
	RightBracket,

	Smaller,
	SmallerEqual,
	Bigger,
	BiggerEqual,
	EqualEqual,
	NotEqual,
	LogicAnd,
	LogicOr,

	BitShiftLeft,
	BitShiftRight,
	And,
	Or,

	EquationSeperator,
	Dot,
} Token_t;

typedef enum {
	ActionGet = 0,
	ActionSet,
	ActionMember,
	ActionCall,
} ActionType_t;

typedef enum {
	ActionExtraNone = 0,
	ActionExtraArrayIndex,
	ActionExtraMemberName,
	ActionExtraCallArgs,
	ActionExtraCallArgsFunction
} ActionExtraType_t;


// Change to a Vector_t with Operator_t's
typedef struct {
	Vector_t operations; // Operation_t. Equations seperated by EquationSep
} Function_t;

struct _ClassFunctionTableEntry_t;

typedef struct _FunctionClass_t {
	union {
		struct {
			u8 builtIn : 1;
		};
	};
	union {
		Function_t function;
		struct _ClassFunctionTableEntry_t* builtInPtr;
	};

} FunctionClass_t;

typedef enum {
	ArrayType_Int = 0,
	ArrayType_String,
	ArrayType_Byte
} ArrayType_t;

typedef struct {
	Vector_t vector; // vector of typeof(value)
	ArrayType_t type : 8;
} ArrayClass_t;

typedef struct _UnsolvedArrayClass_t {
	Vector_t operations; // Operator_t
} UnsolvedArrayClass_t;

typedef struct _DictionaryClass_t {
	Vector_t vector; // vector of typeof(Dict_t)
} DictionaryClass_t;

typedef struct _IntClass_t {
	s64 value;
} IntClass_t;

typedef struct _StringClass_t {
	char* value;
	struct {
		u8 free : 1;
	};
} StringClass_t;

typedef struct _Variable_t {
	//void* variable;
	union {
		FunctionClass_t function;
		UnsolvedArrayClass_t unsolvedArray;
		DictionaryClass_t dictionary;
		IntClass_t integer;
		StringClass_t string;
		ArrayClass_t solvedArray;
	};
	union {
		struct {
			VariableType_t variableType : 4;
			u8 readOnly : 1;
			u8 reference : 1;
		};
	};
} Variable_t;

typedef struct _VariableReference_t {
	union {
		Variable_t* staticVariable;
		char* name;
	};
	void* extra; // Function_t for arrayIdx, char* for member, Function_t for funcCall, Function_t x2 for funcCallArgs
	struct _VariableReference_t* subcall;
	union {
		struct {
			ActionType_t action : 3;
			ActionExtraType_t extraAction : 3;
			u8 staticVariableSet : 1;
			u8 staticVariableRef : 1;
		};
	};
} VariableReference_t;

//typedef Variable_t* (*classFunctionTable)(VariableReference_t*);

typedef Variable_t* (*classFunctionTable)(char*, Variable_t*, VariableReference_t*, Vector_t*);

typedef struct {
	char* name;
	Variable_t* var;
} Dict_t;

typedef struct {
	u8 token : 8;
	char strToken[3];
} TokenConvertion_t;

extern TokenConvertion_t tokenConvertions[];
extern u32 tokenConvertionCount;

typedef struct {
	Vector_t operations; // Operator_t
} Equation_t;

typedef struct {
	Token_t token : 7;
	u8 not : 1;
	union {
		VariableReference_t variable;
		Function_t betweenBrackets;
	};
} Operator_t;

classFunctionTable classTableSearch(u8 classType);
Variable_t* callClass(char* funcName, Variable_t* caller, VariableReference_t* ref, Vector_t* args);

typedef Variable_t* (*ClassFunction)(Variable_t* caller, VariableReference_t* reference, Vector_t* args);

typedef struct _ClassFunctionTableEntry_t {
	char* name;
	ClassFunction func;
	u8 argCount;
	u8* argTypes;
} ClassFunctionTableEntry_t;