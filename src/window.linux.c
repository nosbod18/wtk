#include "../window.h"

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
} G = {0};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Platform functions /////////////////////////////////////////////////////////////////////////////////////////////////

static void default_event_callback(window_t *window, int type, window_event_t const *event) {
    (void)window; (void)type; (void)event;
}

static int platform_start(void) {
    if (!(G.display = XOpenDisplay(NULL)))
        return 0;

    G.screen = DefaultScreen(G.display);
    G.root = RootWindow(G.display, G.screen);
    G.visual = DefaultVisual(G.display, G.screen);

    if (!(G.colormap = XCreateColormap(G.display, G.root, G.visual, AllocNone)))
        return 0;

    G.depth = DefaultDepth(G.display, G.screen);

    if (!(G.wm_delwin = XInternAtom(G.display, "WM_DELETE_WINDOW", 0)))
        return 0;

    G.glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc *)glXGetProcAddressARB((GLubyte const *)"glXCreateContextAttribsARB");

    return 1;
}

static void platform_stop(void) {
    XFreeColormap(G.display, G.colormap);
    XCloseDisplay(G.display);
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

    if (G.nwindows == 0) {
        if (!platform_start())
            return 0;
    }

    window_x11_t *native = malloc(sizeof *native);
    if (!window)
        return 0;

    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = G.colormap
    };

    native->window = XCreateWindow(
        G.display, G.root,
        0, 0, window->w, window->h,
        0, G.depth, InputOutput,
        G.visual, CWColormap | CWEventMask, &swa
    );

    if (!native->window)
        return 0;

    if (!XSetWMProtocols(G.display, native->window, &G.wm_delwin, 1))
        return 0;

    GLint visualAttribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(G.display, G.screen, visualAttribs, &fbcount);
    if (!fbc || !fbcount)
        return 0;

    GLint contextAttribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    if (G.glXCreateContextAttribsARB)
        native->context = G.glXCreateContextAttribsARB(G.display, fbc[0], NULL, 1, contextAttribs);
    else
        native->context = glXCreateNewContext(G.display, fbc[0], GLX_RGBA_TYPE, NULL, 1);

    XMapWindow(G.display, native->window);
    XFlush(G.display);

    window->native = native;
    G.nwindows++;
    return 1;
}

void window_fini(window_t window) {
    window_x11_t *native = window.native;

    if (!native)
        return;

    glXDestroyContext(G.display, native->context);
    XDestroyWindow(G.display, native->window);
    free(native);

    G.nwindows--;
    if (G.nwindows == 0)
        platform_stop();
}

void window_make_current(window_t window) {
    window_x11_t *native = window.native;
    glXMakeContextCurrent(G.display, native->window, native->window, native->context);
}

void window_swap_buffers(window_t window) {
    glXSwapBuffers(G.display, ((window_x11_t *)window.native)->window);
}

void window_poll_events(window_t *window) {
    XEvent event;

    while (XPending(G.display)) {
        XNextEvent(G.display, &event);
        switch (event.type) {
            case ConfigureNotify: {
                // TODO: WINDOWEVENT_MOVE or something like that
                window->w = event.xconfigure.width;
                window->h = event.xconfigure.height;
                window->x = event.xconfigure.x;
                window->y = event.xconfigure.y;
                window->callback(window, WINDOWEVENT_RESIZE, &(window_event_t){0});
            } break;

            case KeyPress:
            case KeyRelease: {
                window->callback(window, event.type == KeyPress ? WINDOWEVENT_KEYDOWN : WINDOWEVENT_KEYUP, &(window_event_t){
                    .key.code = event.xkey.keycode,
                    .key.sym  = event.xkey.keycode,
                    .key.mods = event.xkey.state,
                    .key.x    = event.xkey.x,
                    .key.y    = event.xkey.y
                });
            } break;

            case ButtonPress:
            case ButtonRelease: {
                window->callback(window, event.type == ButtonPress ? WINDOWEVENT_MOUSEDOWN : WINDOWEVENT_MOUSEUP, &(window_event_t){
                    .button.code = event.xbutton.button,
                    .button.sym  = event.xbutton.button,
                    .button.mods = event.xbutton.state,
                    .button.x    = event.xbutton.x,
                    .button.y    = event.xbutton.y
                });
            } break;

            case MotionNotify: {
                window->callback(window, WINDOWEVENT_MOUSEMOTION, &(window_event_t){
                    .motion.dx = event.xmotion.x,
                    .motion.dy = event.xmotion.y,
                });
            } break;

            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == G.wm_delwin) {
                    window->closed = 1; // Must come before window->callback()
                    window->callback(window, WINDOWEVENT_CLOSE, &(window_event_t){0});
                }
            } break;

            case MapNotify:   { window->callback(window, WINDOWEVENT_FOCUSIN, &(window_event_t){0});    } break;
            case UnmapNotify: { window->callback(window, WINDOWEVENT_FOCUSOUT, &(window_event_t){0});   } break;
            case EnterNotify: { window->callback(window, WINDOWEVENT_MOUSEENTER, &(window_event_t){0}); } break;
            case LeaveNotify: { window->callback(window, WINDOWEVENT_MOUSELEAVE, &(window_event_t){0}); } break;

            default: {
            } break;
        }
    }
}

void window_move(window_t *window, int x, int y) {
    XMoveWindow(G.display, ((window_x11_t *)window->native)->window, x, y);
    window->x = x;
    window->y = y;
}

void window_resize(window_t *window, int w, int h) {
    XResizeWindow(G.display, ((window_x11_t *)window->native)->window, (unsigned int)w, (unsigned int)h);
    window->w = w;
    window->h = h;
}

void window_rename(window_t *window, char const *title) {
    XStoreName(G.display, ((window_x11_t *)window->native)->window, title);
    window->title = title;
}

gl_proc_t *window_proc_address(char const *name) {
    return glXGetProcAddress((GLubyte const *)name);
}
