#include "wtk/wtk.h"
#include "platform/platform.h"
#include "plugins/log/log.h"
#include <stdbool.h>
#include <stdlib.h>

Wtk WTK = {0};

static void defaultEventCallback(WtkEvent const *event) {
    (void)event;
}

static bool validateWtkWindowDesc(WtkWindowDesc *desc) {
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

WtkWindow *wtkCreateWindow(WtkWindowDesc *desc) {
    if (!validateWtkWindowDesc(desc))
        return NULL;

    if (WTK.windowCount == 0) {
        if (!platformStart()) {
            error("Failed to start the platform layer");
            platformStop();
            return NULL;
        }
    }

    WtkWindow *window = malloc(sizeof *window);
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
        wtkDeleteWindow(window);
        return NULL;
    }

    WTK.windowCount++;
    return window;
}

void wtkDeleteWindow(WtkWindow *window) {
    if (!window)
        return;

    platformDeleteContext(window);
    platformDeleteWindow(window);
    free(window);

    WTK.windowCount--;
    if (WTK.windowCount == 0)
        platformStop();
}

void wtkMakeCurrent(WtkWindow const *window) {
    platformMakeCurrent(window);
}

void wtkSwapBuffers(WtkWindow const *window) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    platformSwapBuffers(window);
}

void wtkPollEvents(void) {
    platformPollEvents();
}

void wtkGetWindowPos(WtkWindow const *window, int *x, int *y) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    if (x) *x = window->x;
    if (y) *y = window->y;
}

void wtkGetWindowSize(WtkWindow const *window, int *w, int *h) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    if (w) *w = window->w;
    if (h) *h = window->h;
}

int wtkGetWindowShouldClose(WtkWindow const *window) {
    return window ? window->shouldClose : 1;
}

void wtkSetWindowPos(WtkWindow *window, int x, int y) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    platformSetWindowPos(window, x, y);
    window->x = x;
    window->y = y;
}

void wtkSetWindowSize(WtkWindow *window, int w, int h) {
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

void wtkSetWindowTitle(WtkWindow *window, char const *title) {
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

void wtkSetWindowShouldClose(WtkWindow *window, int shouldClose) {
    if (!window) {
        error("Window cannot be NULL");
        return;
    }

    window->shouldClose = shouldClose;
}

WtkGLProc *wtkGetProcAddress(char const *name) {
    if (!name) {
        error("Name cannot be NULL");
        return NULL;
    }

    return platformGetProcAddress(name);
}