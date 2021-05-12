#include "genericClass.h"
#include "model.h"
#include <malloc.h>

Variable_t* copyVariableToPtr(Variable_t var) {
	Variable_t* a = malloc(sizeof(Variable_t));
	*a = var;
	return a;
}