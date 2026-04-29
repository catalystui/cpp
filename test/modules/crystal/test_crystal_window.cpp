#include "modules/crystal/CRYSTAL.h"

struct AppState {
    int running;
};

static AppState appState;

CATALYST_BOOL onWindowClosing(CRYSTALwindow* window) {
    appState.running = 0;
    return CATALYST_FALSE;
}

int main() {
    CATALYST_RESULT result;

    appState.running = 1;

    CRYSTALwindow* window = crystalCreateWindow(&result);

    if (window == 0) {
        return 1;
    }

    crystalSetWindowClosingCallback(window, onWindowClosing);

    while (appState.running) {
        crystalWaitEvents();
    }

    crystalDestroyWindow(window, &result);

    return 0;
}
