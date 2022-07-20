#include "../wtk.h"
#include <stdio.h>

void onEvent(WtkWindow *window, WtkEventType type, WtkEventData const *data) {
    switch (type) {
        case WtkEventType_WindowClose:
            printf("WindowClose={}\n");
            break;
        case WtkEventType_WindowResize:
            printf("WindowResize={.width=%d, .height=%d}\n", data->resize.width, data->resize.height);
            break;
        case WtkEventType_WindowFocus:
            printf("WindowFocus={}\n");
            break;
        case WtkEventType_WindowUnfocus:
            printf("WindowUnfocus={}\n");
            break;
        case WtkEventType_MouseMotion:
            printf("MouseMotion={.button=%d, .mods=%u, .x=%d, .y=%d}\n",
                   data->motion.button, data->motion.mods, data->motion.x, data->motion.y);
            break;
        case WtkEventType_MouseScroll:
            printf("MouseScroll={.dx=%d, .dy=%d}\n", data->scroll.dx, data->scroll.dy);
            break;
        case WtkEventType_MouseEnter:
            printf("MouseEnter={}");
            break;
        case WtkEventType_MouseLeave:
            printf("MouseLeave={}");
            break;
        case WtkEventType_MouseDown:
            printf("MouseDown={.button=%d, .mods=%u, .x=%d, .y=%d}\n",
                   data->button.button, data->button.mods, data->button.x, data->button.y);
            break;
        case WtkEventType_MouseUp:
            printf("MouseDown={.button=%d, .mods=%u, .x=%d, .y=%d}\n",
                   data->button.button, data->button.mods, data->button.x, data->button.y);
            break;
        case WtkEventType_KeyDown:
            printf("KeyDown={.keycode=%d, .scancode=%d, .mods=%u}\n",
                   data->key.keycode, data->key.scancode, data->key.mods);
            break;
        case WtkEventType_KeyUp:
            printf("KeyUp={.keycode=%d, .scancode=%d, .mods=%u}\n",
                   data->key.keycode, data->key.scancode, data->key.mods);
            break;
    }
}

int main(void) {
    if (!wtkInit(&(WtkDesc){0}))
        return 1;

    WtkWindow *window = wtkCreateWindow(&(WtkWindowDesc){.onEvent = onEvent });
    if (!window)
        return 1;

    while (!wtkGetWindowShouldClose(window)) {
        wtkSwapBuffers(window);
        wtkPollEvents();
    }

    wtkDeleteWindow(window);
    wtkQuit();
}

