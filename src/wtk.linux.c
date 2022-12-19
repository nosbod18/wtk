#include "wtk.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include <stdlib.h> // malloc, free
#include <stdio.h>

typedef GLXContext glXCreateContextAttribsARBProc(Display *, GLXFBConfig, GLXContext, Bool, int const *);

typedef struct {
    Window window;
    GLXContext context;
} window_x11_t;

static struct {
    glXCreateContextAttribsARBProc *glXCreateContextAttribsARB;
    Display *display;
    XContext context;
    Visual *visual;
    Window root;
    Colormap colormap;
    Atom wm_delwin;
    int screen;
    int depth;
    int nwindows;
} _wtk = {0};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform functions /////////////////////////////////////////////////////////////////////////////////////////////////

static void default_event_callback(wtk_window_t *window, int type, window_event_t const *event) {
    (void)window; (void)type; (void)event;
}

static int init(void) {
    if (!(_wtk.display = XOpenDisplay(NULL)))
        return 0;

    _wtk.screen = DefaultScreen(_wtk.display);
    _wtk.root   = RootWindow(_wtk.display, _wtk.screen);
    _wtk.visual = DefaultVisual(_wtk.display, _wtk.screen);

    if (!(_wtk.colormap = XCreateColormap(_wtk.display, _wtk.root, _wtk.visual, AllocNone)))
        return 0;

    _wtk.depth = DefaultDepth(_wtk.display, _wtk.screen);

    if (!(_wtk.wm_delwin = XInternAtom(_wtk.display, "WM_DELETE_WINDOW", 0)))
        return 0;

    _wtk.glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc *)glXGetProcAddressARB((GLubyte const *)"glXCreateContextAttribsARB");

    return 1;
}

static void terminate(void) {
    XFreeColormap(_wtk.display, _wtk.colormap);
    XCloseDisplay(_wtk.display);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Window functions ///////////////////////////////////////////////////////////////////////////////////////////////////

int wtk_open_window(wtk_window_t *window) {
    if (!window)
        return 0;

    if (!window->callback)  window->callback = default_event_callback;
    if (!window->title)     window->title = "";
    if (!window->w)         window->w = 640;
    if (!window->h)         window->h = 480;

    if (_wtk.nwindows == 0) {
        if (!init())
            return 0;
    }

    window_x11_t *native = malloc(sizeof *native);
    if (!window)
        return 0;

    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = _wtk.colormap
    };

    native->window = XCreateWindow(
        _wtk.display, _wtk.root,
        0, 0, window->w, window->h,
        0, _wtk.depth, InputOutput,
        _wtk.visual, CWColormap | CWEventMask, &swa
    );

    if (!native->window)
        return 0;

    if (!XSetWMProtocols(_wtk.display, native->window, &_wtk.wm_delwin, 1))
        return 0;

    GLint visualAttribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(_wtk.display, _wtk.screen, visualAttribs, &fbcount);
    if (!fbc || !fbcount)
        return 0;

    GLint contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    if (_wtk.glXCreateContextAttribsARB)
        native->context = _wtk.glXCreateContextAttribsARB(_wtk.display, fbc[0], NULL, 1, contextAttribs);
    else
        native->context = glXCreateNewContext(_wtk.display, fbc[0], GLX_RGBA_TYPE, NULL, 1);

    XMapWindow(_wtk.display, native->window);
    XFlush(_wtk.display);

    window->native = native;
    _wtk.nwindows++;
    return 1;
}

void wtk_close_window(wtk_window_t window) {
    window_x11_t *native = window.native;

    if (!native)
        return;

    glXDestroyContext(_wtk.display, native->context);
    XDestroyWindow(_wtk.display, native->window);
    free(native);

    _wtk.nwindows--;
    if (_wtk.nwindows == 0)
        terminate();
}

void wtk_move_window(wtk_window_t *window, int x, int y) {
    XMoveWindow(_wtk.display, ((window_x11_t *)window->native)->window, x, y);
    window->x = x;
    window->y = y;
}

void wtk_resize_window(wtk_window_t *window, int w, int h) {
    XResizeWindow(_wtk.display, ((window_x11_t *)window->native)->window, (unsigned int)w, (unsigned int)h);
    window->w = w;
    window->h = h;
}

void wtk_rename_window(wtk_window_t *window, char const *title) {
    XStoreName(_wtk.display, ((window_x11_t *)window->native)->window, title);
    window->title = title;
}

void wtk_make_current(wtk_window_t window) {
    window_x11_t *native = window.native;
    glXMakeContextCurrent(_wtk.display, native->window, native->window, native->context);
}

void wtk_swap_buffers(wtk_window_t window) {
    glXSwapBuffers(_wtk.display, ((window_x11_t *)window.native)->window);
}

void wtk_poll_events(wtk_window_t *window) {
    XEvent event;

    while (XPending(_wtk.display)) {
        XNextEvent(_wtk.display, &event);
        switch (event.type) {
            case ConfigureNotify: {
                // TODO: WTK_EVENT_MOVE or something like that
                window->w = event.xconfigure.width;
                window->h = event.xconfigure.height;
                window->x = event.xconfigure.x;
                window->y = event.xconfigure.y;
                window->callback(window, WTK_EVENT_RESIZE, &(window_event_t){0});
            } break;

            case KeyPress:
            case KeyRelease: {
                window->callback(window, event.type == KeyPress ? WTK_EVENT_KEYDOWN : WTK_EVENT_KEYUP, &(window_event_t){
                    .key.code = event.xkey.keycode,
                    .key.sym  = event.xkey.keycode,
                    .key.mods = event.xkey.state,
                    .key.x    = event.xkey.x,
                    .key.y    = event.xkey.y
                });
            } break;

            case ButtonPress:
            case ButtonRelease: {
                window->callback(window, event.type == ButtonPress ? WTK_EVENT_MOUSEDOWN : WTK_EVENT_MOUSEUP, &(window_event_t){
                    .button.code = event.xbutton.button,
                    .button.sym  = event.xbutton.button,
                    .button.mods = event.xbutton.state,
                    .button.x    = event.xbutton.x,
                    .button.y    = event.xbutton.y
                });
            } break;

            case MotionNotify: {
                window->callback(window, WTK_EVENT_MOUSEMOTION, &(window_event_t){
                    .motion.dx = event.xmotion.x,
                    .motion.dy = event.xmotion.y,
                });
            } break;

            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == _wtk.wm_delwin) {
                    window->closed = 1; // Must come before window->callback()
                    window->callback(window, WTK_EVENT_CLOSE, &(window_event_t){0});
                }
            } break;

            case MapNotify:   { window->callback(window, WTK_EVENT_FOCUSIN, &(window_event_t){0});    } break;
            case UnmapNotify: { window->callback(window, WTK_EVENT_FOCUSOUT, &(window_event_t){0});   } break;
            case EnterNotify: { window->callback(window, WTK_EVENT_MOUSEENTER, &(window_event_t){0}); } break;
            case LeaveNotify: { window->callback(window, WTK_EVENT_MOUSELEAVE, &(window_event_t){0}); } break;

            default: {
            } break;
        }
    }
}
