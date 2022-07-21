#include "wtk/wtk.h"

int main(void) {
    wtkInit(&(WtkDesc){0});

    WtkWindow *window = wtkCreateWindow(&(WtkWindowDesc){0});

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
    wtkQuit();
}

