#include "wnd/wnd.h"
#include <stdio.h>

void onEvent(WndEvent const *event) {
    switch (event->type) {
        case WndEventType_WindowClose:
            printf("WindowClose={}\n");
            break;
        case WndEventType_WindowResize: {
            int w, h;
            WndGetWindowSize(event->window, &w, &h);
            printf("WindowResize={.width=%d, .height=%d}\n", w, h);
        } break;
        case WndEventType_WindowFocusIn:
            printf("WindowFocusIn={}\n");
            break;
        case WndEventType_WindowFocusOut:
            printf("WindowFocusOut={}\n");
            break;
        case WndEventType_MouseMotion:
            printf("MouseMotion={.button=%d, .mods=%u, .x=%d, .y=%d}\n", event->button, event->mods, event->location.x, event->location.y);
            break;
        case WndEventType_MouseScroll:
            printf("MouseScroll={.dx=%d, .dy=%d}\n", event->delta.x, event->delta.y);
            break;
        case WndEventType_MouseEnter:
            printf("MouseEnter={}");
            break;
        case WndEventType_MouseLeave:
            printf("MouseLeave={}");
            break;
        case WndEventType_MouseDown:
            printf("MouseDown={.button=%d, .mods=%u, .x=%d, .y=%d}\n", event->button, event->mods, event->location.x, event->location.y);
            break;
        case WndEventType_MouseUp:
            printf("MouseDown={.button=%d, .mods=%u, .x=%d, .y=%d}\n", event->button, event->mods, event->location.x, event->location.y);
            break;
        case WndEventType_KeyDown:
            printf("KeyDown={.keycode=%d, .sym=%d, .mods=%u}\n", event->key, event->sym, event->mods);
            break;
        case WndEventType_KeyUp:
            printf("KeyUp={.keycode=%d, .sym=%d, .mods=%u}\n", event->key, event->sym, event->mods);
            break;
    }
}

int main(void) {
    WndWindow *window = WndCreateWindow(&(WndWindowDesc){.onEvent = onEvent});
    WndMakeCurrent(window);

    while (!WndGetWindowShouldClose(window)) {
        WndSwapBuffers(window);
        WndPollEvents();
    }

    WndDeleteWindow(window);
}

