#include "src/wtk.h"

void callback(WtkWindow *window, WtkEvent const *event) {
    if (event->type == WtkEventType_KeyDown)
        WtkSetWindowShouldClose(window, true);
}

int main(void) {
    WtkWindow *window = WtkCreateWindow(&(WtkWindowDesc){.on_event = callback});
    WtkMakeCurrent(window);

    while (!WtkGetWindowShouldClose(window)) {
        WtkSwapBuffers(window);
        WtkPollEvents();
    }

    WtkDeleteWindow(window);
}
