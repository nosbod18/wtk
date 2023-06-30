# wtk

A single-header cross-platform window and OpenGL context library

```c
// Windows: Link with `-lopengl32 -lgdi32`
// Linux:   Link with `-lX11 -lGL`
// MacOS:   Compile with `-x objective-c` and link with `-framework Cocoa -framework OpenGL`

#define WTK_IMPL
#include "wtk/wtk.h"

void callback(wtk_window_t *window, wtk_event_t const *event) {
    if (event->type == WTK_EVENTTYPE_KEYDOWN)
        wtk_window_set_closed(window, 1);
}

int main(void) {
    wtk_window_t *window = wtk_window_create(&(wtk_window_desc_t){.callback = callback});
    wtk_window_make_current(window);

    while (!wtk_window_closed(window)) {
        wtk_window_swap_buffers(window);
        wtk_poll_events();
    }

    wtk_window_delete(window);
}

```

## License
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
