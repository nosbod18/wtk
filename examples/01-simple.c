#include "wnd/wnd.h"

int main(void) {
    WndWindow *window = WndCreateWindow(&(WndWindowDesc){0});
    WndMakeCurrent(window);

    while (!WndGetWindowShouldClose(window)) {
        WndSwapBuffers(window);
        WndPollEvents();
    }

    WndDeleteWindow(window);
}

