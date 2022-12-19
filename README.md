# Window

## Using
Simply drop the 3 files into your project. Use
```bash
$ wget https://raw.githubusercontent.com/nosbod18/window/source/{window.h,window.linux.c,window.macos.m} {destination}
```

## Example
```c
#include "wtk/wtk.h"

void callback(wtk_window_t *window, int type, wtk_event_t const *event) {
    if (type == WTK_EVENT_KEYDOWN && event->key.code == WTK_KEY_ESCAPE)
        window->closed = 1;
}

int main(void) {
    wtk_window_t window = {
        .title = "Window Test",
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
