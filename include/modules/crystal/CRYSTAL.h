#ifndef CRYSTAL_H
#define CRYSTAL_H
#include "CATCRLIB.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct CRYSTALnative {
    CATALYST_NUINT primary;
    CATALYST_NUINT secondary;
    CATALYST_NUINT tertiary;
} CRYSTALnative;

typedef struct CRYSTALwindow CRYSTALwindow;

CRYSTALwindow* crystalCreateWindow(CATALYST_RESULT* result);

void crystalDestroyWindow(CRYSTALwindow* window);

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* CRYSTAL_H */
