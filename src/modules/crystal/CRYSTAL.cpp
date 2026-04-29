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

void crystalSetWindowClosingCallback(CRYSTALwindow* window, CRYSTALwindowClosingCallback callback) {
    if (window == 0) return;
    window->closingCallback = callback;
}
