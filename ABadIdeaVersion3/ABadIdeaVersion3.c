#include <stdio.h>
#include "compat.h"
#include "parser.h"
#include "intClass.h"
#include "StringClass.h"
#include "eval.h"

int main()
{
    //gfx_printf("Hello world!\n");

    //StartParse("a b c de 0x1 0x10 \"yeet\" + + ");
    /*
    Variable_t b = newStringVariable("Hello world\n", 1, 0);
    callClass("__print__", &b, NULL, NULL);

    Variable_t a = newIntVariable(69, 0);
    Variable_t c = newIntVariable(1, 0);
    Variable_t e = newStringVariable("snake", 1, 0);
    Vector_t fuk = newVec(sizeof(Variable_t), 1);
    vecAdd(&fuk, c);
    Variable_t *d = callClass("+", &a, NULL, &fuk);
    callClass("__print__", d, NULL, NULL);
    */
    
    //parseScript("#REQUIRE VER 3.0.5\nmain = { two = 1 + 1 }");
    //ParserRet_t ret = parseScript("a.b.c(1){ a.b.c() }");
    ParserRet_t ret = parseScript("(1 + 2).print()");

    setStaticVars(&ret.staticVarHolder);
    Variable_t* res = eval(ret.main.operations.data, ret.main.operations.count, 1);

    //gfx_printf("\na %d\n", getIntValue(&a));

    /*
    Variable_t a = newIntVariable(69, 0);
    Variable_t b = newIntVariable(1, 0);
    VariableReference_t ref = { .action = ActionGet, .extraAction = ActionExtraMemberName, .extra = "+" };
    Variable_t* c = genericGet(&a, &ref);
    Variable_t* args[] = { &b };
    Variable_t* d = genericCallDirect(c, args, 1);

    ref.extra = "__print__";
    Variable_t* e = genericGet(d, &ref);
    Variable_t* f = genericCallDirect(e, NULL, 0);
    */

}