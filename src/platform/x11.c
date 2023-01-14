#include "wtk/wtk.h"
#include "platform.h"
#include <X11/Xutil.h>

typedef GLXContext GlXCreateContextAttribsARBProc(Display *, GLXFBConfig, GLXContext, Bool, int const *);

static GlXCreateContextAttribsARBProc  *g_glx_create_ctx_attribs = 0;
static Display                         *g_display = 0;
static XContext                         g_context = {0};
static Visual                          *g_visual = 0;
static Window                           g_root = 0;
static Colormap                         g_colormap = {0};
static Atom                             g_wm_delwin = 0;
static int                              g_screen = 0;
static int                              g_depth = 0;

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
    if (!(g_display = XOpenDisplay(NULL)))
        return false;

    g_screen = DefaultScreen(g_display);
    g_root   = RootWindow(g_display, g_screen);
    g_visual = DefaultVisual(g_display, g_screen);

    if (!(g_colormap = XCreateColormap(g_display, g_root, g_visual, AllocNone)))
        return false;

    g_depth = DefaultDepth(g_display, g_screen);

    if (!(g_wm_delwin = XInternAtom(g_display, "WM_DELETE_WINDOW", 0)))
        return false;

    g_glx_create_ctx_attribs = (GlXCreateContextAttribsARBProc *)glXGetProcAddressARB((GLubyte const *)"glx_create_ctx_attribs");

    return true;
}

void platformTerminate(void) {
    XFreeColormap(g_display, g_colormap);
    XCloseDisplay(g_display);
}

bool platformCreateWindow(WtkWindow *window) {
    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = g_colormap
    };

    window->window = XCreateWindow(
        g_display, g_root,
        window->desc.x, window->desc.y, window->desc.w, window->desc.h,
        0, g_depth, InputOutput,
        g_visual, CWColormap | CWEventMask, &swa
    );
    if (!native->window) return false;

    if (!XSetWMProtocols(g_display, native->window, &g_wm_delwin, 1))
        return false;

    GLint vis_attribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(g_display, g_screen, vis_attribs, &fbcount);
    if (!fbc || !fbcount) return false;

    GLint ctx_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    if (g_glx_create_ctx_attribs)
        native->context = g_glx_create_ctx_attribs(g_display, fbc[0], NULL, 1, ctx_attribs);
    else
        native->context = glXCreateNewContext(g_display, fbc[0], GLX_RGBA_TYPE, NULL, 1);

    XMapWindow(g_display, native->window);
    XFlush(g_display);
    return true;
}

void platformMakeCurrent(WtkWindow *window) {
    glXMakeContextCurrent(g_display, window->window, window->window, window->context);
}

void platformSwapBuffers(WtkWindow *window) {
    glXSwapBuffers(g_display, window->window);
}

void platformPollEvents(void) {
    XEvent event;
    while (XPending(g_display)) {
        XNextEvent(g_display, &event);
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
                if ((Atom)event.xclient.data.l[0] == g_wm_delwin) {
                    WtkSetWindowShouldClose(window, true);
                    postOtherEvent(window, WtkEventType_WindowClose);
                }
            } break;
        }
    }
}

void platformDeleteWindow(WtkWindow *window) {
    glXDestroyContext(g_display, window->context);
    XDestroyWindow(g_display, window->window);
}

void platformSetWindowOrigin(WtkWindow *window, int x, int y) {
    XMoveWindow(g_display, window->window, x, y);
}

void platformSetWindowSize(WtkWindow *window, int w, int h) {
    XResizeWindow(g_display, window->window, (unsigned int)w, (unsigned int)h);
}

void platformSetWindowTitle(WtkWindow *window, char const *title) {
    XStoreName(g_display, window->window, title);
}
