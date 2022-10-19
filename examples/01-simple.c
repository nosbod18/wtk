#include "wtk/wtk.h"

int main(void) {
    WtkWindow *window = wtkCreateWindow(&(WtkWindowDesc){0});
    wtkMakeCurrent(window);

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
}

