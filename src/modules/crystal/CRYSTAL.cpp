#include "CMAKECFG.h"
#include "CATCRLIB.hpp"
#include "CATMMLIB.hpp"
#include "CATTELIB.hpp"
#include "CRYSTAL.internal.h"

using namespace catalyst;

void crystalPollEvents() {
    crystalProcessEvents(FALSE);
}

void crystalWaitEvents() {
    crystalProcessEvents(TRUE);
}

void crystalSetWindowErroredCallback(CRYSTALwindow* window, CRYSTALwindowErroredCallback callback) {
    if (window == 0) return;
    window->erroredCallback = callback;
}

void crystalSetWindowRepositionedCallback(CRYSTALwindow* window, CRYSTALwindowRepositionedCallback callback) {
    if (window == 0) return;
    window->repositionedCallback = callback;
}

void crystalSetWindowResizedCallback(CRYSTALwindow* window, CRYSTALwindowResizedCallback callback) {
    if (window == 0) return;
    window->resizedCallback = callback;
}

void crystalSetWindowRefreshCallback(CRYSTALwindow* window, CRYSTALwindowRefreshCallback callback) {
    if (window == 0) return;
    window->refreshCallback = callback;
}

void crystalSetWindowRedrawCallback(CRYSTALwindow* window, CRYSTALwindowRedrawCallback callback) {
    if (window == 0) return;
    window->redrawCallback = callback;
}

void crystalSetWindowFocusedCallback(CRYSTALwindow* window, CRYSTALwindowFocusedCallback callback) {
    if (window == 0) return;
    window->focusedCallback = callback;
}

void crystalSetWindowUnfocusedCallback(CRYSTALwindow* window, CRYSTALwindowUnfocusedCallback callback) {
    if (window == 0) return;
    window->unfocusedCallback = callback;
}

void crystalSetWindowMinimizedCallback(CRYSTALwindow* window, CRYSTALwindowMinimizedCallback callback) {
    if (window == 0) return;
    window->minimizedCallback = callback;
}

void crystalSetWindowMaximizedCallback(CRYSTALwindow* window, CRYSTALwindowMaximizedCallback callback) {
    if (window == 0) return;
    window->maximizedCallback = callback;
}

void crystalSetWindowRestoredCallback(CRYSTALwindow* window, CRYSTALwindowRestoredCallback callback) {
    if (window == 0) return;
    window->restoredCallback = callback;
}

void crystalSetWindowShownCallback(CRYSTALwindow* window, CRYSTALwindowShownCallback callback) {
    if (window == 0) return;
    window->shownCallback = callback;
}

void crystalSetWindowHiddenCallback(CRYSTALwindow* window, CRYSTALwindowHiddenCallback callback) {
    if (window == 0) return;
    window->hiddenCallback = callback;
}

void crystalSetWindowClosingCallback(CRYSTALwindow* window, CRYSTALwindowClosingCallback callback) {
    if (window == 0) return;
    window->closingCallback = callback;
}
