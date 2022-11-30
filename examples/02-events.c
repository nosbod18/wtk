#include "../window.h"
#include <stdio.h>

void event(window_t *window, int type, window_event_t const *event) {
    switch (type) {
        case EVENTTYPE_WINDOWCLOSE:
            printf("WindowClose={}\n");
            break;
        case EVENTTYPE_WINDOWRESIZE: {
            printf("WindowResize={.width=%d, .height=%d}\n", window->w, window->h);
        } break;
        case EVENTTYPE_WINDOWFOCUSIN:
            printf("WindowFocusIn={}\n");
            break;
        case EVENTTYPE_WINDOWFOCUSOUT:
            printf("WindowFocusOut={}\n");
            break;
        case EVENTTYPE_MOUSEMOTION:
            printf("MouseMotion={.dx=%d, .dy=%d}\n", event->motion.dx, event->motion.dy);
            break;
        case EVENTTYPE_MOUSESCROLL:
            printf("MouseScroll={.dx=%d, .dy=%d}\n", event->scroll.dx, event->scroll.dy);
            break;
        case EVENTTYPE_MOUSEENTER:
            printf("MouseEnter={}");
            break;
        case EVENTTYPE_MOUSELEAVE:
            printf("MouseLeave={}");
            break;
        case EVENTTYPE_MOUSEDOWN:
            printf("MouseDown={.button=%d, .sym=%d, .mods=%u, .x=%d, .y=%d}\n",
                   event->button.code, event->button.sym, event->button.mods, event->button.x, event->button.y);
            break;
        case EVENTTYPE_MOUSEUP:
            printf("MouseUp={.button=%d, .sym=%d, .mods=%u, .x=%d, .y=%d}\n",
                   event->button.code, event->button.sym, event->button.mods, event->button.x, event->button.y);
            break;
        case EVENTTYPE_KEYDOWN:
            printf("KeyDown={.key=%d, .sym=%d, .mods=%u, .x=%d, .y=%d}\n",
                   event->key.code, event->key.sym, event->key.mods, event->key.x, event->key.y);
            break;
        case EVENTTYPE_KEYUP:
            printf("KeyUp={.key=%d, .sym=%d, .mods=%u, .x=%d, .y=%d}\n",
                   event->key.code, event->key.sym, event->key.mods, event->key.x, event->key.y);
            break;
    }
}

int main(void) {
    window_t window = {.callback = event};

    window_init(&window);
    window_make_current(window);

    while (!window.closed) {
        window_swap_buffers(window);
        window_poll_events(&window);
    }

    window_fini(window);
}

