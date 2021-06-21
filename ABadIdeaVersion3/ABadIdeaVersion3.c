#include <stdio.h>
#include "compat.h"
#include "parser.h"
#include "intClass.h"
#include "StringClass.h"
#include "eval.h"
#include "garbageCollector.h"

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
    /*
    Vector_t v = newVec(sizeof(int), 4);
    int a = 69;
    vecAdd(&v, a);
    vecAdd(&v, a);
    vecAdd(&v, a);
    vecAdd(&v, a);

    vecForEach(int*, b, (&v))
        printf("%d\n", *b);

    return;
    */
    
    initGarbageCollector();

    //parseScript("#REQUIRE VER 3.0.5\nmain = { two = 1 + 1 }");
    //ParserRet_t ret = parseScript("a.b.c(1){ a.b.c() }");
    ParserRet_t ret = parseScript(" a = 0 while(1) { print(a, \"\\r\") a = a + 1 } ");

    setStaticVars(&ret.staticVarHolder);
    initRuntimeVars();
    Variable_t* res = eval(ret.main.operations.data, ret.main.operations.count, 1);

    exitGarbageCollector();

}