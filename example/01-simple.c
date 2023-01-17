#include "wtk/wtk.h"

int main(void) {
    WtkWindow *window = WtkCreateWindow(&(WtkWindowDesc){0});
    WtkMakeCurrent(window);

    while (!WtkGetWindowShouldClose(window)) {
        WtkSwapBuffers(window);
        WtkPollEvents();
    }

    WtkDeleteWindow(window);
}
