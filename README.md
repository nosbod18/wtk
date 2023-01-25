# wtk

A small, simple, cross-platform window and OpenGL context library

```c
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
```

## Building
Simply add the src/ files to your project and list them in your build system. Be sure to link against `-lX11 -lGL` on Linux and `-framework Cocoa -framework OpenGL` on MacOS.

`wtk` only supports MacOS and Linux right now as those are the platforms I use, but I want to get a Windows backend up and running at some point.
