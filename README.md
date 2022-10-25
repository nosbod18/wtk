# Wtk - Window toolkit
Wtk is a simple platform-independent window and OpenGL context library. It provides a unified API for window creation, manipulation, and input handling.

## Supported platforms
- MacOS (Cocoa)
- Linux (X11)

## Planned platforms
- Windows (Win32)

## Building
### MacOS and Linux
Run `make` to build the library and examples, or `make libwtk.a` to only build the library. Use `make run-{example file without extension}` to run an example, e.g. `make run-01-simple` to run the first example.

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
