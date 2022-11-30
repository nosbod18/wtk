# Window

## Building
### MacOS and Linux
Build the library and the examples:
```bash
$ make
```

Build just the library:
```bash
$ make libwindow.a
```

Run an example:
```bash
$ make run-{example file without extension}
```
- e.g. `$ make run-01-basic` to compile and run `examples/01-basic.c`.

### Windows
Currently doesn't support Windows

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
