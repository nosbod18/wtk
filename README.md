# Wtk - Window toolkit
Wtk is a simple platform-independent window and OpenGL context library. It provides a unified API for window creation, manipulation, and input handling.

## Supported platforms
- MacOS (Cocoa)

## Planned platforms
- Linux (Xlib, XCB, Wayland)
- Windows (Win32)

## Building
### Unix
```bash
$ ./build.sh [--option[=<value>]]
```

### Windows
**TODO**

## Example
```c
#include "wtk/wtk.h"

int main(void) {
    wtkInit(&(WtkDesc){0});

    WtkWindow *window = wtkCreateWindow(&(WtkWindowDesc){0});

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
    wtkQuit();
}
```

See the [examples](https://www.github.com/nosbod18/wtk/tree/main/examples) directory for more
