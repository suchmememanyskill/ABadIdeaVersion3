#include "model.h"
#include "vector.h"
#include "genericClass.h"
#include "compat.h"

typedef struct {
	Variable_t* ref;
	u16 refCount;
} ReferenceCounter_t;

Vector_t pendingAdd = { 0 };
Vector_t pendingRemove = { 0 };
Vector_t storedReferences = { 0 };

void initGarbageCollector() {
	pendingAdd = newVec(sizeof(Variable_t*), 4);
	pendingRemove = newVec(sizeof(Variable_t*), 4);
	storedReferences = newVec(sizeof(ReferenceCounter_t), 8);
}

void addPendingReference(Variable_t* ref) {
	if (ref == NULL)
		return;

	if (!ref->gcDoNotFree)
		vecAdd(&pendingAdd, ref);
}

void removePendingReference(Variable_t* ref) {
	if (ref == NULL)
		return;

	if (!ref->gcDoNotFree) {
		if (ref->variableType == FunctionClass && ref->function.builtIn) {
			removePendingReference(ref->function.origin);
		}

		if (ref->variableType == SolvedArrayReferenceClass) {
			removePendingReference(ref->solvedArray.arrayClassReference);
		}
		vecAdd(&pendingRemove, ref);
	}
		
}

void modReference(Variable_t* ref, u8 add) {
	vecForEach(ReferenceCounter_t*, references, (&storedReferences)) {
		if (references->ref == ref) {
			if (add)
				references->refCount++;
			else {
				if (--references->refCount <= 0) {
					freeVariable(&references->ref);
					vecRem(&storedReferences, ((u8*)references - (u8*)storedReferences.data) / storedReferences.elemSz);
				}
			}

			return;
		}
	}
	
	if (!add)
		return;

	ReferenceCounter_t r = { .ref = ref, .refCount = 1 };
	vecAdd(&storedReferences, r);
}

void processPendingReferences() {
	vecForEach(Variable_t**, references, (&pendingAdd))
		modReference(*references, 1);

	vecForEach(Variable_t**, references, (&pendingRemove))
		modReference(*references, 0);
}

void exitGarbageCollector() {
	processPendingReferences();

	vecForEach(ReferenceCounter_t*, references, (&storedReferences)) {
		gfx_printf("[WARN] referenced var %p at exit\n", references->ref);
		freeVariable(&references->ref);
	}

	vecFree(pendingAdd);
	vecFree(pendingRemove);
	vecFree(storedReferences);
}