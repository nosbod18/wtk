# Window

## Using
Simply drop the 3 files into your project. Use
```bash
$ wget https://raw.githubusercontent.com/nosbod18/window/source/{window.h,window.linux.c,window.macos.m} {destination}
```

## Example
```c
#include "window/window.h"

void callback(window_t *window, int type, window_event_t const *event) {
    if (type == WINDOWEVENT_KEYDOWN && event->key.code == KEY_ESCAPE)
        window->closed = 1;
}

int main(void) {
    window_t window = {
        .title = "Window Test",
        .callback = callback
    };

    window_init(&window);
    window_make_current(window);

    while (!window.closed) {
        window_swap_buffers(window);
        window_poll_events(&window);
    }

    window_fini(window);
}
```