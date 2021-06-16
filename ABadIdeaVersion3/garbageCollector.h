#include "model.h"

void initGarbageCollector();
void addPendingReference(Variable_t* ref);
void processPendingReferences();
void exitGarbageCollector();
void removePendingReference(Variable_t* ref);