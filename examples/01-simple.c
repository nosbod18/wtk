#include "../wtk.h"

int main(void) {
    if (!wtkInit(&(WtkDesc){0}))
        return 1;

    WtkWindow *window = wtkCreateWindow(&(WtkWindowDesc){0});
    if (!window)
        return 1;

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
    wtkQuit();
}

