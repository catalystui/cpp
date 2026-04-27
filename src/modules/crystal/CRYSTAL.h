#ifndef CRYSTAL_H
#define CRYSTAL_H
#include "CATCRLIB.h"
#ifdef __cplusplus
extern "C" {
namespace catalyst {
namespace modules {
namespace crystal {
#endif

typedef struct CRYSTALnative {
    NUINT primary;
    NUINT secondary;
    NUINT tertiary;
} CRYSTALnative;

typedef struct CRYSTALwindow CRYSTALwindow;

#ifdef __cplusplus
} /* namespace crystal */
} /* namespace modules */
} /* namespace catalyst */
} /* extern "C" */
#endif
#endif /* CRYSTAL_H */
