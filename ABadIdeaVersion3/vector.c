#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Vector_t newVec(u8 typesz, u32 preallocate) {
	Vector_t res = {
		.data = calloc(preallocate, typesz),
		.capacity = preallocate * typesz,
		.count = 0,
		.elemSz = typesz
	};
	// check .data != null;
	return res;
}

Vector_t vecFromArray(void* array, u32 count, u32 typesz)
{
	Vector_t res = {
		.data = array,
		.capacity = count * typesz,
		.count = count,
		.elemSz = typesz
	};
	return res;
}

int vecAddElem(Vector_t* v, void* elem, u8 sz) {
	if (!v || !elem || v->elemSz != sz)
		return 0;

	u32 usedbytes = v->count * sz;
	if (usedbytes >= v->capacity)
	{
		v->capacity *= 2;
		void* buff = malloc(v->capacity);
		if (!buff)
			return 0;
		memcpy(buff, v->data, v->capacity / 2);
		free(v->data);
		v->data = buff;
	}

	memcpy((char*)v->data + usedbytes, elem, sz);
	v->count++;
	return 1;
}

Vector_t vecCopyOffset(Vector_t* orig, u32 offset) {
	Vector_t dst = newVec(orig->elemSz, orig->count - offset);
	memcpy(dst.data, ((u8*)orig->data + orig->elemSz * offset), (orig->count - offset) * orig->elemSz);
	dst.count = orig->count - offset;
	return dst;
}

Vector_t vecCopy(Vector_t* orig) {
	return vecCopyOffset(orig, 0);
}


void* getStackEntry(Vector_t *stack) {
	if (stack->count <= 0)
		return NULL;

	return ((u8*)stack->data + (stack->elemSz * (stack->count - 1)));
}

// This will stay valid until the queue is modified
void* popStackEntry(Vector_t* stack) {
	if (stack->count <= 0)
		return NULL;

	void* a = getStackEntry(stack);
	stack->count--;
	return a;
}