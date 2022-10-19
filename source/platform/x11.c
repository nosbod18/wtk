#include "wtk/wtk.h"
#include "platform.h"

#define GLAD_GLX_IMPLEMENTATION
#include "plugins/glad/glx.h"
#include "plugins/log/log.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdlib.h> // malloc, free

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions ///////////////////////////////////////////////////////////////////////////////////////////////////

static int translateKey(unsigned int sym) {
    return sym; // TODO
}

static int translateButton(unsigned int mask) {
    return mask; // TODO
}

static unsigned int translateMods(unsigned int mask) {
    return mask; // TODO
}

static void postInputEvent(WtkWindow *window, WtkEventType type, XEvent const *event) {
    window->onEvent(&(WtkEvent){
        .window = window,
        .type = type,
        .time = event->xkey.time,
        .location = {
            .x = event->xkey.x,
            .y = event->xkey.y,
        },
        .delta = {0}, // TODO
        .key = translateKey(event->xkey.keycode),
        .button = translateButton(event->xkey.state),
        .sym = event->xkey.keycode,
        .mods = translateMods(event->xkey.state),
        .isRepeat = 0, // TODO
        .clicks = 1, // TODO
    });
}

static void postOtherEvent(WtkWindow *window, WtkEventType type, XEvent const *event) {
    window->onEvent(&(WtkEvent){
        .window = window,
        .type = type,
        .time = event->xkey.time
    });
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Internal platform functions ////////////////////////////////////////////////////////////////////////////////////////

static int platformStart(void) {
    if (!(WTK.x11.display = XOpenDisplay(NULL))) {
        error("Failed to open X display");
        return 0;
    }

    WTK.x11.context = XUniqueContext();
    WTK.x11.screen = DefaultScreen(WTK.x11.display);
    WTK.x11.root = RootWindow(WTK.x11.display, WTK.x11.screen);
    WTK.x11.visual = DefaultVisual(WTK.x11.display, WTK.x11.screen);

    if (!(WTK.x11.colormap = XCreateColormap(WTK.x11.display, WTK.x11.root, WTK.x11.visual, AllocNone))) {
        error("Failed to create colormap");
        return 0;
    }

    WTK.x11.depth = DefaultDepth(WTK.x11.display, WTK.x11.screen);

    if (!(WTK.x11.wmDeleteWindow = XInternAtom(WTK.x11.display, "WM_DELETE_WINDOW", False))) {
        error("Failed to intern WM_DELETE_WINDOW atom");
        return 0;
    }

    if (!gladLoaderLoadGLX(WTK.x11.display, WTK.x11.screen)) {
        error("Failed to load GLX");
        return 0;
    }

    return 1;
}

static void platformStop(void) {
    XFreeColormap(WTK.x11.display, WTK.x11.colormap);
    XCloseDisplay(WTK.x11.display);
    gladLoaderUnloadGLX();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// External platform functions ////////////////////////////////////////////////////////////////////////////////////////

int platformCreateWindow(WtkWindow *window) {
    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = WTK.x11.colormap
    };

    window->x11.window = XCreateWindow(
        WTK.x11.display, WTK.x11.root,
        0, 0, window->w, window->h,
        0, WTK.x11.depth, InputOutput,
        WTK.x11.visual, CWColormap | CWEventMask, &swa
    );

    if (!window->x11.window) {
        error("Failed to create X11 window");
        return 0;
    }

    if (!XSetWMProtocols(WTK.x11.display, window->x11.window, &WTK.x11.wmDeleteWindow, 1)) {
        error("Failed to set window manager protocols");
        return 0;
    }

    if (XSaveContext(WTK.x11.display, window->x11.window, WTK.x11.context, (XPointer)window)) {
        error("Failed to save X context");
        return 0;
    }

    XMapWindow(WTK.x11.display, window->x11.window);
    XFlush(WTK.x11.display);

    return 1;
}

void platformDeleteWindow(WtkWindow *window) {
    XDestroyWindow(WTK.x11.display, window->x11.window);
}

int platformCreateContext(WtkWindow *window, WtkWindowDesc const *desc) {
    GLint visualAttribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(WTK.x11.display, WTK.x11.screen, visualAttribs, &fbcount);

    if (!fbc || !fbcount) {
        error("Failed to find suitable a framebuffer config");
        return 0;
    }

    GLint contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, desc->context.major,
        GLX_CONTEXT_MINOR_VERSION_ARB, desc->context.minor,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    window->x11.context = glXCreateContextAttribsARB(WTK.x11.display, fbc[0], NULL, 1, contextAttribs);
    return 1;
}

void platformDeleteContext(WtkWindow *window) {
    glXDestroyContext(WTK.x11.display, window->x11.context);
}

void platformMakeCurrent(WtkWindow const *window) {
    if (window)
        glXMakeContextCurrent(WTK.x11.display, window->x11.window, window->x11.window, window->x11.context);
    else
        glXMakeContextCurrent(WTK.x11.display, 0, 0, NULL);
}

void platformSwapBuffers(WtkWindow const *window) {
    glXSwapBuffers(WTK.x11.display, window->x11.window);
}

void platformPollEvents(void) {
    WtkWindow *window;
    XEvent event;

    while (XPending(WTK.x11.display)) {
        XNextEvent(WTK.x11.display, &event);
        if (XFindContext(WTK.x11.display, event.xany.window, WTK.x11.context, (XPointer *)&window))
            continue;

        switch (event.type) {
            case ConfigureNotify:   postOtherEvent(window, WtkEventType_WindowResize, &event);      break;
            case MapNotify:         postOtherEvent(window, WtkEventType_WindowFocusIn, &event);     break;
            case UnmapNotify:       postOtherEvent(window, WtkEventType_WindowFocusOut, &event);    break;
            case KeyPress:          postInputEvent(window, WtkEventType_KeyDown, &event);           break;
            case KeyRelease:        postInputEvent(window, WtkEventType_KeyUp, &event);             break;
            case ButtonPress:       postInputEvent(window, WtkEventType_MouseDown, &event);         break;
            case ButtonRelease:     postInputEvent(window, WtkEventType_MouseUp, &event);           break;
            case MotionNotify:      postInputEvent(window, WtkEventType_MouseMotion, &event);       break;
            case EnterNotify:       postOtherEvent(window, WtkEventType_MouseEnter, &event);        break;
            case LeaveNotify:       postOtherEvent(window, WtkEventType_MouseLeave, &event);        break;
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == WTK.x11.wmDeleteWindow) {
                    // wtkSetWindowShouldClose must come first since the user may decide to cancel the close request when they handle the event
                    wtkSetWindowShouldClose(window, 1);
                    postOtherEvent(window, WtkEventType_WindowClose, &event);
                }
                break;
            default:
                break;
        }
    }
}

void platformSetWindowPos(WtkWindow *window, int x, int y) {
    XMoveWindow(WTK.x11.display, window->x11.window, x, y);
}

void platformSetWindowSize(WtkWindow *window, int w, int h) {
    XResizeWindow(WTK.x11.display, window->x11.window, (unsigned int)w, (unsigned int)h);
}

void platformSetWindowTitle(WtkWindow *window, char const *title) {
    XStoreName(WTK.x11.display, window->x11.window, title);
}
