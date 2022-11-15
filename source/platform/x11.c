#if defined(__linux__)

#include "wnd/wnd.h"
#include "platform.h"

#include "plugins/log/log.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <stdbool.h>
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

static void postInputEvent(WndWindow *window, WndEventType type, XEvent const *event) {
    window->onEvent(&(WndEvent){
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

static void postOtherEvent(WndWindow *window, WndEventType type, XEvent const *event) {
    window->onEvent(&(WndEvent){
        .window = window,
        .type = type,
        .time = event->xkey.time
    });
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform functions /////////////////////////////////////////////////////////////////////////////////////////////////

bool platformStart(void) {
    if (!(_wnd.x11.display = XOpenDisplay(NULL))) {
        error("Failed to open X display");
        return false;
    }

    _wnd.x11.context = XUniqueContext();
    _wnd.x11.screen = DefaultScreen(_wnd.x11.display);
    _wnd.x11.root = RootWindow(_wnd.x11.display, _wnd.x11.screen);
    _wnd.x11.visual = DefaultVisual(_wnd.x11.display, _wnd.x11.screen);

    if (!(_wnd.x11.colormap = XCreateColormap(_wnd.x11.display, _wnd.x11.root, _wnd.x11.visual, AllocNone))) {
        error("Failed to create colormap");
        return false;
    }

    _wnd.x11.depth = DefaultDepth(_wnd.x11.display, _wnd.x11.screen);

    if (!(_wnd.x11.wmDeleteWindow = XInternAtom(_wnd.x11.display, "WM_DELETE_WINDOW", False))) {
        error("Failed to intern WM_DELETE_WINDOW atom");
        return false;
    }

    _wnd.x11.glXCreateContextAttribsARB = (void *)glXGetProcAddressARB((GLubyte const *)"glXCreateContextAttribsARB");

    return true;
}

void platformStop(void) {
    XFreeColormap(_wnd.x11.display, _wnd.x11.colormap);
    XCloseDisplay(_wnd.x11.display);
}

bool platformCreateWindow(WndWindow *window) {
    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = _wnd.x11.colormap
    };

    window->x11.window = XCreateWindow(
        _wnd.x11.display, _wnd.x11.root,
        0, 0, window->w, window->h,
        0, _wnd.x11.depth, InputOutput,
        _wnd.x11.visual, CWColormap | CWEventMask, &swa
    );

    if (!window->x11.window) {
        error("Failed to create X11 window");
        return false;
    }

    if (!XSetWMProtocols(_wnd.x11.display, window->x11.window, &_wnd.x11.wmDeleteWindow, 1)) {
        error("Failed to set window manager protocols");
        return false;
    }

    if (XSaveContext(_wnd.x11.display, window->x11.window, _wnd.x11.context, (XPointer)window)) {
        error("Failed to save X context");
        return false;
    }

    XMapWindow(_wnd.x11.display, window->x11.window);
    XFlush(_wnd.x11.display);

    return true;
}

void platformDeleteWindow(WndWindow *window) {
    XDestroyWindow(_wnd.x11.display, window->x11.window);
}

bool platformCreateContext(WndWindow *window, WndWindowDesc const *desc) {
    GLint visualAttribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(_wnd.x11.display, _wnd.x11.screen, visualAttribs, &fbcount);

    if (!fbc || !fbcount) {
        error("Failed to find suitable a framebuffer config");
        return false;
    }

    GLint contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, desc->context.major,
        GLX_CONTEXT_MINOR_VERSION_ARB, desc->context.minor,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    if (!_wnd.x11.glXCreateContextAttribsARB) {
        error("Could not use glXCreateContextAttribsARB, expect the wrong version of OpenGL");
        window->x11.context = glXCreateNewContext(_wnd.x11.display, fbc[0], GLX_RGBA_TYPE, NULL, True);
    } else {
        window->x11.context = _wnd.x11.glXCreateContextAttribsARB(_wnd.x11.display, fbc[0], NULL, 1, contextAttribs);
    }

    return true;
}

void platformDeleteContext(WndWindow *window) {
    glXDestroyContext(_wnd.x11.display, window->x11.context);
}

void platformMakeCurrent(WndWindow const *window) {
    if (window)
        glXMakeContextCurrent(_wnd.x11.display, window->x11.window, window->x11.window, window->x11.context);
    else
        glXMakeContextCurrent(_wnd.x11.display, 0, 0, NULL);
}

void platformSwapBuffers(WndWindow const *window) {
    glXSwapBuffers(_wnd.x11.display, window->x11.window);
}

void platformPollEvents(void) {
    WndWindow *window;
    XEvent event;

    while (XPending(_wnd.x11.display)) {
        XNextEvent(_wnd.x11.display, &event);
        if (XFindContext(_wnd.x11.display, event.xany.window, _wnd.x11.context, (XPointer *)&window))
            continue;

        switch (event.type) {
            case ConfigureNotify:   postOtherEvent(window, WndEventType_WindowResize, &event);      break;
            case MapNotify:         postOtherEvent(window, WndEventType_WindowFocusIn, &event);     break;
            case UnmapNotify:       postOtherEvent(window, WndEventType_WindowFocusOut, &event);    break;
            case KeyPress:          postInputEvent(window, WndEventType_KeyDown, &event);           break;
            case KeyRelease:        postInputEvent(window, WndEventType_KeyUp, &event);             break;
            case ButtonPress:       postInputEvent(window, WndEventType_MouseDown, &event);         break;
            case ButtonRelease:     postInputEvent(window, WndEventType_MouseUp, &event);           break;
            case MotionNotify:      postInputEvent(window, WndEventType_MouseMotion, &event);       break;
            case EnterNotify:       postOtherEvent(window, WndEventType_MouseEnter, &event);        break;
            case LeaveNotify:       postOtherEvent(window, WndEventType_MouseLeave, &event);        break;
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == _wnd.x11.wmDeleteWindow) {
                    // WndSetWindowShouldClose must come first since the user may decide to cancel the close request when they handle the event
                    WndSetWindowShouldClose(window, 1);
                    postOtherEvent(window, WndEventType_WindowClose, &event);
                }
                break;
            default:
                break;
        }
    }
}

void platformSetWindowPos(WndWindow *window, int x, int y) {
    XMoveWindow(_wnd.x11.display, window->x11.window, x, y);
}

void platformSetWindowSize(WndWindow *window, int w, int h) {
    XResizeWindow(_wnd.x11.display, window->x11.window, (unsigned int)w, (unsigned int)h);
}

void platformSetWindowTitle(WndWindow *window, char const *title) {
    XStoreName(_wnd.x11.display, window->x11.window, title);
}

WndGLProc *platformGetProcAddress(char const *name) {
    return glXGetProcAddress((GLubyte const *)name);
}

#endif // __linux__
