# wtk

A small, simple, cross-platform window and OpenGL context library

```c
#include "wtk.h"

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

```

## Building
Simply drop the files under `wtk/` to your project and add them to your build system

### Linux
Link with `-lX11 -lGL`

### MacOS
Compile with `-x objective-c` and link with `-framework Cocoa -framework OpenGL`

### Windows
Unfortunately, wtk only supports Linux and MacOS right now

