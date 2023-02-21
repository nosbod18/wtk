# wtk

A single-header cross-platform window and OpenGL context library

```c
// Windows: Link with `-lopengl32 -lgdi32`
// Linux:   Link with `-lX11 -lGL`
// MacOS:   Compile with `-x objective-c` and link with `-framework Cocoa -framework OpenGL`

#define WTK_IMPLEMENTATION
#include "wtk/wtk.h"

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