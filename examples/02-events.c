#include "wtk/wtk.h"
#include <stdio.h>

void onEvent(WtkEvent const *event) {
    switch (event->type) {
        case WtkEventType_WindowClose:
            printf("WindowClose={}\n");
            break;
        case WtkEventType_WindowResize: {
            int w, h;
            wtkGetWindowSize(event->window, &w, &h);
            printf("WindowResize={.width=%d, .height=%d}\n", w, h);
        } break;
        case WtkEventType_WindowFocusIn:
            printf("WindowFocusIn={}\n");
            break;
        case WtkEventType_WindowFocusOut:
            printf("WindowFocusOut={}\n");
            break;
        case WtkEventType_MouseMotion:
            printf("MouseMotion={.button=%d, .mods=%u, .x=%d, .y=%d}\n", event->button, event->mods, event->location.x, event->location.y);
            break;
        case WtkEventType_MouseScroll:
            printf("MouseScroll={.dx=%d, .dy=%d}\n", event->delta.x, event->delta.y);
            break;
        case WtkEventType_MouseEnter:
            printf("MouseEnter={}");
            break;
        case WtkEventType_MouseLeave:
            printf("MouseLeave={}");
            break;
        case WtkEventType_MouseDown:
            printf("MouseDown={.button=%d, .mods=%u, .x=%d, .y=%d}\n", event->button, event->mods, event->location.x, event->location.y);
            break;
        case WtkEventType_MouseUp:
            printf("MouseDown={.button=%d, .mods=%u, .x=%d, .y=%d}\n", event->button, event->mods, event->location.x, event->location.y);
            break;
        case WtkEventType_KeyDown:
            printf("KeyDown={.keycode=%d, .sym=%d, .mods=%u}\n", event->key, event->sym, event->mods);
            break;
        case WtkEventType_KeyUp:
            printf("KeyUp={.keycode=%d, .sym=%d, .mods=%u}\n", event->key, event->sym, event->mods);
            break;
    }
}

int main(void) {
    WtkWindow *window = wtkCreateWindow(&(WtkWindowDesc){.onEvent = onEvent});
    wtkMakeCurrent(window);

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
}

