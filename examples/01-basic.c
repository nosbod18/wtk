#include "../window.h"

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

