# wtk - Window toolkit

## About
A simple cross platform window library with an API very similar to [GLFW](https://www.github.com/glfw/glfw).
Provides functions to open a window and create an OpenGL context, and process events.

## Example
```c
#include "wtk/wtk.h"

int main(void) {
    WtkWindow *window = WtkCreateWindow(&(WtkWindowDesc){0});

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
}
```

See the [examples](https://www.github.com/nosbod18/wtk/tree/main/examples) for more


## Building
#### \*nix
```bash
$ ./build.sh [--backend=<option>]
```

#### Windows
TODO

## Supported platforms
- MacOS (Cocoa)

## Planned platforms
- Linux (X11, XCB, Wayland)
- Windows (Win32)
- Web (Emscripten)
