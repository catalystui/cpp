#ifndef CRYSTAL_INTERNAL_H
#define CRYSTAL_INTERNAL_H
#include "modules/crystal/CRYSTAL.h"
#ifdef __cplusplus
extern "C" {
#endif

struct CRYSTALwindow {
    CRYSTALnative native; /* Native handles */
    void* platform; /* Platform-specific pointer */

    /* Callbacks */
    CRYSTALwindowErroredCallback erroredCallback;
    CRYSTALwindowRepositionedCallback repositionedCallback;
    CRYSTALwindowResizedCallback resizedCallback;
    CRYSTALwindowRefreshCallback refreshCallback;
    CRYSTALwindowRedrawCallback redrawCallback;
    CRYSTALwindowFocusedCallback focusedCallback;
    CRYSTALwindowUnfocusedCallback unfocusedCallback;
    CRYSTALwindowMinimizedCallback minimizedCallback;
    CRYSTALwindowMaximizedCallback maximizedCallback;
    CRYSTALwindowRestoredCallback restoredCallback;
    CRYSTALwindowShownCallback shownCallback;
    CRYSTALwindowHiddenCallback hiddenCallback;
    CRYSTALwindowClosingCallback closingCallback;
};

/* Internal platform/backend event pump. Not part of the public CRYSTAL API. */
/* No support will be provided if this method is used outside of internal code. */
CRYSTAL_API void crystalProcessEvents(CATALYST_BOOL wait);

#ifdef __cplusplus
} /* extern C */
#endif
#endif /* CRYSTAL_INTERNAL_H */
