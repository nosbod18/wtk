#include "../wtk.h"

int main(void) {
    WtkWindow *window = wtkCreateWindow(&(WtkWindowDesc){0});

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
}

