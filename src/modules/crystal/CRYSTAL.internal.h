#ifndef CRYSTAL_INTERNAL_H
#define CRYSTAL_INTERNAL_H
#include "modules/crystal/CRYSTAL.h"

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

void crystalProcessEvents(CATALYST_BOOL wait);

#endif /* CRYSTAL_INTERNAL_H */
