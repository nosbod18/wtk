#define WTK_IMPL
#include "wtk.h"

int main(void) {
    wtk_window *window = wtk_window_create(&(wtk_window_desc){0});
    wtk_make_current(window);

    while (!wtk_window_closed(window)) {
        wtk_swap_buffers(window);
        wtk_poll_events();
    }

    wtk_window_delete(window);
}
