# Wtk - Window toolkit

## About
A simple cross platform window library with an API similar to [GLFW](https://www.github.com/glfw/glfw).
Provides functions to open a window, manipulate windows, and process events.

## Example
```c
#include "wtk/wtk.h"

int main(void) {
    if (!wtkInit(&(WtkDesc){0}))
        return 1;

    WtkWindow *window = wtkCreateWindow(&(WtkWindowDesc){0});
    if (!window)
        return false;

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
    wtkQuit();
}
```

See the [examples](https://www.github.com/nosbod18/wtk/tree/main/examples) for more


## Building
- Unix: ` $ ./build.sh [--backend=<option>]`
- Windows: TODO

## Supported platforms
- MacOS (Cocoa)

## Planned platforms
- Linux (X11, XCB, Wayland)
- Windows (Win32)
