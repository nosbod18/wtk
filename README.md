# Window
A C99 OpenGL window library

## Building
### MacOS and Linux
#### `$ make`
- Builds the library and the examples.

#### `$ make libwindow.a`
- Builds only the library.

#### `$ make run-{example file without extension}`
- Runs an example, e.g. `make run-01-basic` compiles and runs `examples/01-basic.c`.
- Execute `./.build/$(OS)-$(ARCH)-$(MODE)/bin/{example file without extension}` if you don't want to use `make run`. See the `Makefile` for what `$(OS)`, `$(ARCH)`, and `$(MODE)` are.

### Windows
Currently cannot build on Windows

## Example
```c
#include "window/window.h"

int main(void) {
    window_t window = {0};

    window_init(&window);
    window_make_current(window);

    while (!window.closed) {
        window_swap_buffers(window);
        window_poll_events(&window);
    }

    window_fini(window);
}
```

See the [examples](https://www.github.com/nosbod18/window/tree/main/examples) directory for more
