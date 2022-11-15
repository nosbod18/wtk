#include "wnd/wnd.h"
#include "platform/platform.h"
#include "plugins/log/log.h"
#include <stdbool.h>
#include <stdlib.h>

Wnd _wnd = {0};

static void defaultEventCallback(WndEvent const *event) {
    (void)event;
}

static bool validateWndWindowDesc(WndWindowDesc *desc) {
    if (!desc)
        return false;

    if (!desc->context.major)   desc->context.major = 4;
    if (!desc->context.minor)   desc->context.minor = 5;
    if (!desc->onEvent)         desc->onEvent       = defaultEventCallback;
    if (!desc->title)           desc->title         = "";
    if (!desc->width)           desc->width         = 640;
    if (!desc->height)          desc->height        = 480;

    return true;
}

WndWindow *WndCreateWindow(WndWindowDesc *desc) {
    if (!validateWndWindowDesc(desc))
        return NULL;

    if (_wnd.windowCount == 0) {
        if (!platformStart()) {
            error("Failed to start the platform layer");
            platformStop();
            return NULL;
        }
    }

    WndWindow *window = malloc(sizeof *window);
    if (!window) {
        error("Failed to allocate window");
        return NULL;
    }

    window->onEvent = desc->onEvent;
    window->title = desc->title;
    window->w = desc->width;
    window->h = desc->height;
    window->shouldClose = 0;

    if (!platformCreateWindow(window) || !platformCreateContext(window, desc)) {
        WndDeleteWindow(window);
        return NULL;
    }

    _wnd.windowCount++;
    return window;
}

void WndDeleteWindow(WndWindow *window) {
    if (!window)
        return;

    platformDeleteContext(window);
    platformDeleteWindow(window);
    free(window);

    _wnd.windowCount--;
    if (_wnd.windowCount == 0)
        platformStop();
}

void WndMakeCurrent(WndWindow const *window) {
    platformMakeCurrent(window);
}

void WndSwapBuffers(WndWindow const *window) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    platformSwapBuffers(window);
}

void WndPollEvents(void) {
    platformPollEvents();
}

void WndGetWindowPos(WndWindow const *window, int *x, int *y) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    if (x) *x = window->x;
    if (y) *y = window->y;
}

void WndGetWindowSize(WndWindow const *window, int *w, int *h) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    if (w) *w = window->w;
    if (h) *h = window->h;
}

int WndGetWindowShouldClose(WndWindow const *window) {
    return window ? window->shouldClose : 1;
}

void WndSetWindowPos(WndWindow *window, int x, int y) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    platformSetWindowPos(window, x, y);
    window->x = x;
    window->y = y;
}

void WndSetWindowSize(WndWindow *window, int w, int h) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    if (w <= 0 || h <= 0) {
        error("Size must be greater than 0 (got %d, %d)", w, h);
        return;
    }

    platformSetWindowSize(window, w, h);
    window->w = w;
    window->h = h;
}

void WndSetWindowTitle(WndWindow *window, char const *title) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    if (!title) {
        error("Title cannot be NULL");
        return;
    }

    platformSetWindowTitle(window, title);
    window->title = title;
}

void WndSetWindowShouldClose(WndWindow *window, int shouldClose) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    window->shouldClose = shouldClose;
}

WndGLProc *WndGetProcAddress(char const *name) {
    if (!name) {
        error("Name cannot be NULL");
        return NULL;
    }

    return platformGetProcAddress(name);
}
