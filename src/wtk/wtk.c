#include "wtk/wtk.h"
#include "platform/platform.h"
#include <stdlib.h> // calloc, free

static void defaultEventCallback(WtkWindow *window, WtkEventType type, WtkEvent const *event) {
    (void)window; (void)type; (void)event;
}

static void validateDesc(WtkWindow *window) {
    WtkWindowDesc *d = &window->desc;
    if (!d->event_handler)  d->event_handler = defaultEventCallback;
    if (!d->title)          d->title = "";
    if (!d->w)              d->w = 640;
    if (!d->h)              d->h = 480;
}

WtkWindow *WtkCreateWindow(WtkWindowDesc const *desc) {
    if (!desc || (_wtk.num_windows == 0 && !platformInit()))
        return NULL;

    WtkWindow *window = calloc(1, sizeof *window);
    if (!window) return NULL;

    window->desc = *desc;
    validateDesc(window);

    if (!platformCreateWindow(window)) {
        WtkDeleteWindow(window);
        return NULL;
    }

    _wtk.num_windows++;
    return window;
}

void WtkMakeCurrent(WtkWindow *window) {
    if (window) platformMakeCurrent(window);
}

void WtkSwapBuffers(WtkWindow *window) {
    if (window) platformSwapBuffers(window);
}

void WtkPollEvents(void) {
    platformPollEvents();
}

void WtkDeleteWindow(WtkWindow *window) {
    if (!window) return;

    platformDeleteWindow(window);
    free(window);
    if (--_wtk.num_windows == 0)
        platformTerminate();
}

void WtkGetWindowRect(WtkWindow const *window, int *x, int *y, int *w, int *h) {
    if (x) *x = window ? window->desc.x : -1;
    if (y) *y = window ? window->desc.y : -1;
    if (w) *w = window ? window->desc.w : -1;
    if (h) *h = window ? window->desc.h : -1;
}

bool WtkGetWindowShouldClose(WtkWindow const *window) {
    return window ? window->closed : true;
}

void WtkSetWindowOrigin(WtkWindow *window, int x, int y) {
    if (!window || x < 0 || y < 0) return;

    platformSetWindowOrigin(window, x, y);
    window->desc.x = x;
    window->desc.y = y;
}

void WtkSetWindowSize(WtkWindow *window, int w, int h) {
    if (!window || w < 0 || h < 0) return;

    platformSetWindowSize(window, w, h);
    window->desc.w = w;
    window->desc.h = h;
}

void WtkSetWindowTitle(WtkWindow *window, char const *title) {
    if (!window || !title) return;

    platformSetWindowTitle(window, title);
    window->desc.title = title;
}

void WtkSetWindowShouldClose(WtkWindow *window, bool should_close) {
    if (window)
        window->closed = should_close;
}