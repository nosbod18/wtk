#include "wtk/wtk.h"
#include "platform.h"

static void postKeyOrButtonEvent(WtkWindow *window, WtkEventType type, XEvent const *event) {
    bool isKeyEvent = (type == WtkEventType_KeyDown || type == WtkEventType_KeyUp);
    window->desc.event_handler(window, type, &(WtkEvent){
        .key.code = isKeyEvent ? event->xkey.keycode : event->xbutton.button,
        .key.sym  = isKeyEvent ? event->xkey.keycode : event->xbutton.button,
        .key.mods = isKeyEvent ? event->xkey.state   : event->xbutton.state,
        .key.y    = isKeyEvent ? event->xkey.x       : event->xbutton.x,
        .key.x    = isKeyEvent ? event->xkey.y       : event->xbutton.y
    });
}

static void postMotionOrScrollEvent(WtkWindow *window, WtkEventType type, XEvent const *event) {
    bool isMotionEvent = (type == WtkEventType_MouseMotion);
    window->desc.event_handler(window, type, &(WtkEvent){
        .motion.dx = isMotionEvent ? event->xmotion.x : event->xscroll.x,
        .motion.dy = isMotionEvent ? event->xmotion.y : event->xscroll.y
    });
}

static void postOtherEvent(WtkWindow *window, WtkEventType type) {
    window->desc.event_handler(window, type, &(WtkEvent){0});
}

bool platformInit(void) {
    if (!(_wtk.display = XOpenDisplay(NULL)))
        return false;

    _wtk.screen = DefaultScreen(_wtk.display);
    _wtk.root   = RootWindow(_wtk.display, _wtk.screen);
    _wtk.visual = DefaultVisual(_wtk.display, _wtk.screen);

    if (!(_wtk.colormap = XCreateColormap(_wtk.display, _wtk.root, _wtk.visual, AllocNone)))
        return false;

    _wtk.depth = DefaultDepth(_wtk.display, _wtk.screen);

    if (!(_wtk.wm_delwin = XInternAtom(_wtk.display, "WM_DELETE_WINDOW", 0)))
        return false;

    _wtk.glx_create_ctx_attribs = (GlXCreateContextAttribsARBProc *)glXGetProcAddressARB((GLubyte const *)"glx_create_ctx_attribs");

    return true;
}

void platformTerminate(void) {
    XFreeColormap(_wtk.display, _wtk.colormap);
    XCloseDisplay(_wtk.display);
}

bool platformCreateWindow(WtkWindow *window) {
    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = _wtk.colormap
    };

    window->window = XCreateWindow(
        _wtk.display, _wtk.root,
        window->desc.x, window->desc.y, window->desc.w, window->desc.h,
        0, _wtk.depth, InputOutput,
        _wtk.visual, CWColormap | CWEventMask, &swa
    );
    if (!native->window) return false;

    if (!XSetWMProtocols(_wtk.display, native->window, &_wtk.wm_delwin, 1))
        return false;

    GLint vis_attribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(_wtk.display, _wtk.screen, vis_attribs, &fbcount);
    if (!fbc || !fbcount) return false;

    GLint ctx_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    if (_wtk.glx_create_ctx_attribs)
        native->context = _wtk.glx_create_ctx_attribs(_wtk.display, fbc[0], NULL, 1, ctx_attribs);
    else
        native->context = glXCreateNewContext(_wtk.display, fbc[0], GLX_RGBA_TYPE, NULL, 1);

    XMapWindow(_wtk.display, native->window);
    XFlush(_wtk.display);
    return true;
}

void platformMakeCurrent(WtkWindow *window) {
    glXMakeContextCurrent(_wtk.display, window->window, window->window, window->context);
}

void platformSwapBuffers(WtkWindow *window) {
    glXSwapBuffers(_wtk.display, window->window);
}

void platformPollEvents(void) {
    XEvent event;
    while (XPending(_wtk.display)) {
        XNextEvent(_wtk.display, &event);
        switch (event.type) {
            case KeyPress:      postKeyOrButtonEvent(window, WtkEventType_KeyDown, &event);         break;
            case KeyRelease:    postKeyOrButtonEvent(window, WtkEventType_KeyUp, &event);           break;
            case ButtonPress:   postKeyOrButtonEvent(window, WtkEventType_MouseDown, &event);       break;
            case ButtonRelease: postKeyOrButtonEvent(window, WtkEventType_MouseUp, &event);         break;
            case MotionNotify:  postMotionOrScrollEvent(window, WtkEventType_MouseMotion, &event);  break;
            case MapNotify:     postOtherEvent(window, WtkEventType_WindowFocusIn);                 break;
            case UnmapNotify:   postOtherEvent(window, WtkEventType_WindowFocusOut);                break;
            case EnterNotify:   postOtherEvent(window, WtkEventType_WindowMouseEnter);              break;
            case LeaveNotify:   postOtherEvent(window, WtkEventType_WindowMouseLeave);              break;
            case ConfigureNotify: {
                window->desc.x = event.xconfigure.x;
                window->desc.y = event.xconfigure.y;
                window->desc.w = event.xconfigure.width;
                window->desc.h = event.xconfigure.height;
                postOtherEvent(window, WtkEventType_WindowResize);
            } break;
            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == _wtk.wm_delwin) {
                    WtkSetWindowShouldClose(window, true);
                    postOtherEvent(window, WtkEventType_WindowClose);
                }
            } break;
        }
    }
}

void platformDeleteWindow(WtkWindow *window) {
    glXDestroyContext(_wtk.display, window->context);
    XDestroyWindow(_wtk.display, window->window);
}

void platformSetWindowOrigin(WtkWindow *window, int x, int y) {
    XMoveWindow(_wtk.display, window->window, x, y);
}

void platformSetWindowSize(WtkWindow *window, int w, int h) {
    XResizeWindow(_wtk.display, window->window, (unsigned int)w, (unsigned int)h);
}

void platformSetWindowTitle(WtkWindow *window, char const *title) {
    XStoreName(_wtk.display, window->window, title);
}
