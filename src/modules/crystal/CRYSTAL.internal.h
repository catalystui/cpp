#ifndef CRYSTAL_INTERNAL_H
#define CRYSTAL_INTERNAL_H
#include "modules/crystal/CRYSTAL.h"

struct CRYSTALwindow {
    CRYSTALnative native; /* Native handles */
    void* platform; /* Platform-specific pointer */
    CRYSTALwindowClosingCallback closingCallback; /* Window closing callback */
};

void crystalProcessEvents(CATALYST_BOOL wait);

#endif /* CRYSTAL_INTERNAL_H */
