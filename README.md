# Wtk - Window toolkit
Wtk is a simple platform-independent window and OpenGL context library. It provides a unified API for window creation, manipulation, and input handling.

## Supported platforms
- MacOS (Cocoa)
- Linux (X11)

## Planned platforms
- Windows (Win32)

## Building
### Unix
```bash
$ make
```
This will compile the Wtk library and the examples

### Windows
Currently cannot build on Windows

## Example
```c
#include "wtk/wtk.h"

int main(void) {
    WtkWindow *window = wtkCreateWindow(&(WtkWindowDesc){0});
    wtkMakeCurrent(window);

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
}
```

See the [examples](https://www.github.com/nosbod18/wtk/tree/main/examples) directory for more
