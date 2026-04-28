#ifndef CRYSTAL_H
#define CRYSTAL_H
#include "CATCRLIB.h"
#ifdef __cplusplus
extern "C" {
#endif

#define CRYSTAL_DEFAULT_WIDTH   800
#define CRYSTAL_DEFAULT_HEIGHT  450
#define CRYSTAL_DEFAULT_TITLE   "Crystal Window"

typedef struct CRYSTALnative {
    CATALYST_NUINT primary;
    CATALYST_NUINT secondary;
    CATALYST_NUINT tertiary;
} CRYSTALnative;

typedef struct CRYSTALwindow CRYSTALwindow;

CRYSTALwindow* crystalCreateWindow(CATALYST_RESULT* result);

void crystalDestroyWindow(CRYSTALwindow* window, CATALYST_RESULT* result);

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* CRYSTAL_H */
