#include <stdio.h>
#include "compat.h"
#include "parser.h"
#include "intClass.h"
#include "StringClass.h"

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
    
    Function_t* thing = StartParse("#REQUIRE VER 3.0.5\nmain = { two = 1 + 1 }");

    //gfx_printf("\na %d\n", getIntValue(&a));
}