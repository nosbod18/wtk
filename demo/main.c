#include "../wtk.h"

void callback(WtkWindow *window, WtkEvent const *event) {
    if (event->type == WTK_EVENTTYPE_KEYDOWN)
        WtkSetWindowShouldClose(window, 1);
}

int main(void) {
    WtkWindow *window = WtkCreateWindow(&(WtkWindowDesc){.callback = callback});
    WtkMakeCurrent(window);

    while (!WtkGetWindowShouldClose(window)) {
        WtkSwapBuffers(window);
        WtkPollEvents();
    }

    WtkDeleteWindow(window);
}
