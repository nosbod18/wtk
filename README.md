# wtk

A small, simple, cross-platform window and OpenGL context library

```c
#include "wtk/wtk.h"

void callback(wtk_window_t *window, int type, wtk_event_t const *event) {
    if (type == WTK_EVENT_KEYDOWN && event->key.code == WTK_KEY_ESCAPE)
        window->closed = 1;
}

int main(void) {
    wtk_window_t window = {
        .title = "Callback Test",
        .callback = callback
    };

    wtk_open_window(&window);
    wtk_make_current(window);

    while (!window.closed) {
        wtk_swap_buffers(window);
        wtk_poll_events(&window);
    }

    wtk_close_window(window);
}
```
