#include "../wtk.h"

WtkWindow *wtkCreateWindow(WtkWindowDesc const *desc) {
    (void)desc;
    return 0;
}

void wtkDeleteWindow(WtkWindow *window) {
    (void)window;
}

void wtkPollEvents(void) {
}

void wtkSwapBuffers(WtkWindow const *window) {
    (void)window;
}

void wtkGetWindowPos(WtkWindow const *window, int *x, int *y) {
    (void)window;
    if (x) *x = 0;
    if (y) *y = 0;
}

void wtkGetWindowSize(WtkWindow const *window, int *width, int *height) {
    (void)window;
    if (width) *width = 0;
    if (height) *height = 0;
}

bool wtkGetWindowVisible(WtkWindow const *window) {
    (void)window;
    return false;
}

bool wtkGetWindowFullscreen(WtkWindow const *window) {
    (void)window;
    return false;
}

bool wtkGetWindowShouldClose(WtkWindow const *window) {
    (void)window;
    return true;
}

void wtkSetWindowPos(WtkWindow *window, int x, int y) {
    (void)window; (void)x; (void)y;
}

void wtkSetWindowSize(WtkWindow *window, int width, int height) {
    (void)window; (void)width; (void)height;
}

void wtkSetWindowTitle(WtkWindow *window, char const *title) {
    (void)window; (void)title;
}

void wtkSetWindowVisible(WtkWindow *window, bool visible) {
    (void)window; (void)visible;
}

void wtkSetWindowFullscreen(WtkWindow *window, bool fullscreen) {
    (void)window; (void)fullscreen;
}

void wtkSetWindowShouldClose(WtkWindow *window, bool shouldClose) {
    (void)window; (void)shouldClose;
}

