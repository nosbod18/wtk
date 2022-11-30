#include "window.h"
#include "log/log.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include <stdbool.h>
#include <stdlib.h> // malloc, free

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
} _wnd = {0};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform functions /////////////////////////////////////////////////////////////////////////////////////////////////

static void default_event_callback(window_t *window, int type, window_event_t const *event) {
    (void)window; (void)type; (void)event;
}

static int platform_init(void) {
    if (!(_wnd.display = XOpenDisplay(NULL))) {
        error("Failed to open X display");
        return 0;
    }

    _wnd.screen = DefaultScreen(_wnd.display);
    _wnd.root = RootWindow(_wnd.display, _wnd.screen);
    _wnd.visual = DefaultVisual(_wnd.display, _wnd.screen);

    if (!(_wnd.colormap = XCreateColormap(_wnd.display, _wnd.root, _wnd.visual, AllocNone))) {
        error("Failed to create colormap");
        return 0;
    }

    _wnd.depth = DefaultDepth(_wnd.display, _wnd.screen);

    if (!(_wnd.wm_delwin = XInternAtom(_wnd.display, "WM_DELETE_WINDOW", 0))) {
        error("Failed to intern WM_DELETE_WINDOW atom");
        return 0;
    }

    _wnd.glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc *)glXGetProcAddressARB((GLubyte const *)"glXCreateContextAttribsARB");

    return 1;
}

static void platform_fini(void) {
    XFreeColormap(_wnd.display, _wnd.colormap);
    XCloseDisplay(_wnd.display);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Window functions ///////////////////////////////////////////////////////////////////////////////////////////////////

int window_init(window_t *window) {
    if (!window)
        return 0;

    if (!window->callback)  window->callback = default_event_callback;
    if (!window->title)     window->title = "";
    if (!window->w)         window->w = 640;
    if (!window->h)         window->h = 480;

    if (_wnd.nwindows == 0) {
        if (!platform_init())
            return 0;
    }

    window_x11_t *native = malloc(sizeof *native);
    if (!window) {
        error("Failed to allocate window");
        return 0;
    }

    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = _wnd.colormap
    };

    native->window = XCreateWindow(
        _wnd.display, _wnd.root,
        0, 0, window->w, window->h,
        0, _wnd.depth, InputOutput,
        _wnd.visual, CWColormap | CWEventMask, &swa
    );

    if (!native->window) {
        error("Failed to create X11 window");
        return 0;
    }

    if (!XSetWMProtocols(_wnd.display, native->window, &_wnd.wm_delwin, 1)) {
        error("Failed to set window manager protocols");
        return 0;
    }

    GLint visualAttribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(_wnd.display, _wnd.screen, visualAttribs, &fbcount);

    if (!fbc || !fbcount) {
        error("Failed to find suitable a framebuffer config");
        return 0;
    }

    GLint contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    if (!_wnd.glXCreateContextAttribsARB) {
        error("Could not use glXCreateContextAttribsARB, expect the wrong version of OpenGL");
        native->context = glXCreateNewContext(_wnd.display, fbc[0], GLX_RGBA_TYPE, NULL, 1);
    } else {
        native->context = _wnd.glXCreateContextAttribsARB(_wnd.display, fbc[0], NULL, 1, contextAttribs);
    }

    XMapWindow(_wnd.display, native->window);
    XFlush(_wnd.display);

    window->native = native;
    _wnd.nwindows++;
    return 1;
}

void window_fini(window_t window) {
    window_x11_t *native = window.native;

    if (!native)
        return;

    glXDestroyContext(_wnd.display, native->context);
    XDestroyWindow(_wnd.display, native->window);
    free(native);

    _wnd.nwindows--;
    if (_wnd.nwindows == 0)
        platform_fini();
}

void window_make_current(window_t window) {
    window_x11_t *native = window.native;
    glXMakeContextCurrent(_wnd.display, native->window, native->window, native->context);
}

void window_swap_buffers(window_t window) {
    glXSwapBuffers(_wnd.display, ((window_x11_t *)window.native)->window);
}

void window_poll_events(window_t *window) {
    XEvent event;

    while (XPending(_wnd.display)) {
        XNextEvent(_wnd.display, &event);
        switch (event.type) {
            case ConfigureNotify: {
                window->w = event.xconfigure.width;
                window->h = event.xconfigure.height;
                window->x = event.xconfigure.x;
                window->y = event.xconfigure.y;
                window->callback(window, EVENTTYPE_WINDOWRESIZE, &(window_event_t){0});
            } break;

            case KeyPress:
            case KeyRelease: {
                window->callback(window, event.type == KeyPress ? EVENTTYPE_KEYDOWN : EVENTTYPE_KEYUP, &(window_event_t){
                    .key.code = event.xkey.keycode,
                    .key.sym  = event.xkey.keycode,
                    .key.mods = event.xkey.state,
                    .key.x    = event.xkey.x,
                    .key.y    = event.xkey.y
                });
            } break;

            case ButtonPress:
            case ButtonRelease: {
                window->callback(window, event.type == ButtonPress ? EVENTTYPE_MOUSEDOWN : EVENTTYPE_MOUSEUP, &(window_event_t){
                    .button.code = event.xbutton.button,
                    .button.sym  = event.xbutton.button,
                    .button.mods = event.xbutton.state,
                    .button.x    = event.xbutton.x,
                    .button.y    = event.xbutton.y
                });
            } break;

            case MotionNotify: {
                window->callback(window, EVENTTYPE_MOUSEMOTION, &(window_event_t){
                    .motion.dx = event.xmotion.x,
                    .motion.dy = event.xmotion.y,
                });
            } break;

            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == _wnd.wm_delwin) {
                    window->closed = 1; // Must come before window->callback()
                    window->callback(window, EVENTTYPE_WINDOWCLOSE, &(window_event_t){0});
                }
            } break;

            case MapNotify:   { window->callback(window, EVENTTYPE_WINDOWFOCUSIN, &(window_event_t){0});  } break;
            case UnmapNotify: { window->callback(window, EVENTTYPE_WINDOWFOCUSOUT, &(window_event_t){0}); } break;
            case EnterNotify: { window->callback(window, EVENTTYPE_MOUSEENTER, &(window_event_t){0});     } break;
            case LeaveNotify: { window->callback(window, EVENTTYPE_MOUSELEAVE, &(window_event_t){0});     } break;

            default: {
            } break;
        }
    }
}

void window_move(window_t *window, int x, int y) {
    XMoveWindow(_wnd.display, ((window_x11_t *)window->native)->window, x, y);
    window->x = x;
    window->y = y;
}

void window_resize(window_t *window, int w, int h) {
    XResizeWindow(_wnd.display, ((window_x11_t *)window->native)->window, (unsigned int)w, (unsigned int)h);
    window->w = w;
    window->h = h;
}

void window_rename(window_t *window, char const *title) {
    XStoreName(_wnd.display, ((window_x11_t *)window->native)->window, title);
    window->title = title;
}

gl_proc_t *window_proc_address(char const *name) {
    return glXGetProcAddress((GLubyte const *)name);
}
