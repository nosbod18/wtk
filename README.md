# Wnd - OpenGL Windowing library
Wnd is a simple platform-independent window and OpenGL context library. It provides a unified API for window creation and input handling.

## Supported platforms
- MacOS (Cocoa)
- Linux (X11)

## Planned platforms
- Windows (Win32)

## Building
### MacOS and Linux
#### `$ make`
- Builds the library and the examples

#### `$ make libwnd.a`
- Builds only the library

#### `$ make run-{example file without extension}`
- Runs an example, e.g. `make run-01-simple` runs the first example.

### Windows
Currently cannot build on Windows

## Example
```c
#include "wnd/wnd.h"

int main(void) {
    WndWindow *window = WndCreateWindow(&(WndWindowDesc){0});
    WndMakeCurrent(window);

    while (!WndGetWindowShouldClose(window)) {
        WndSwapBuffers(window);
        WndPollEvents();
    }

    WndDeleteWindow(window);
}
```

See the [examples](https://www.github.com/nosbod18/wnd/tree/main/examples) directory for more
