#include "src/wtk.h"

void callback(WtkWindow *window, WtkEventType type, WtkEvent const *event) {
    if (type == WtkEventType_KeyDown)
        WtkSetWindowShouldClose(window, true);
}

int main(void) {
    WtkWindow *window = WtkCreateWindow(&(WtkWindowDesc){.event_handler = callback});
    WtkMakeCurrent(window);

    while (!WtkGetWindowShouldClose(window)) {
        WtkSwapBuffers(window);
        WtkPollEvents();
    }

    WtkDeleteWindow(window);
}
