///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                               Interface                                 ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

#pragma once

///////////////////////////////////////////////////////////////////////////////
/// Constants

enum {
    WTK_EVENTTYPE_KEYUP,
    WTK_EVENTTYPE_KEYDOWN,
    WTK_EVENTTYPE_MOUSEUP,
    WTK_EVENTTYPE_MOUSEDOWN,
    WTK_EVENTTYPE_MOUSEENTER,
    WTK_EVENTTYPE_MOUSELEAVE,
    WTK_EVENTTYPE_MOUSESCROLL,
    WTK_EVENTTYPE_MOUSEMOTION,
    WTK_EVENTTYPE_WINDOWCLOSE,
    WTK_EVENTTYPE_WINDOWRESIZE,
    WTK_EVENTTYPE_WINDOWFOCUSIN,
    WTK_EVENTTYPE_WINDOWFOCUSOUT,
};

// Ascii keys can use their character representation, e.g. 'w', 'A', '+', ...
enum {
    WTK_KEY_BACKSPACE = 0x08, WTK_KEY_TAB, WTK_KEY_ENTER, WTK_KEY_ESCAPE = 0x1b,
    WTK_KEY_UP = 0x80, WTK_KEY_DOWN, WTK_KEY_LEFT, WTK_KEY_RIGHT,
    WTK_KEY_PAGEUP, WTK_KEY_PAGEDOWN, WTK_KEY_HOME, WTK_KEY_END, WTK_KEY_INSERT, WTK_KEY_DELETE,
    WTK_KEY_F1, WTK_KEY_F2, WTK_KEY_F3, WTK_KEY_F4, WTK_KEY_F5, WTK_KEY_F6, WTK_KEY_F7, WTK_KEY_F8, WTK_KEY_F9, WTK_KEY_F10, WTK_KEY_F11, WTK_KEY_F12,
    WTK_KEY_LSHIFT, WTK_KEY_LCTRL, WTK_KEY_LSUPER, WTK_KEY_LALT,
    WTK_KEY_RSHIFT, WTK_KEY_RCTRL, WTK_KEY_RSUPER, WTK_KEY_RALT,
    WTK_KEY_CAPSLOCK,
};

enum {
    WTK_BUTTON_1,
    WTK_BUTTON_2,
    WTK_BUTTON_3,
    WTK_BUTTON_4,
    WTK_BUTTON_5,
    WTK_BUTTON_6,
    WTK_BUTTON_7,
    WTK_BUTTON_8,
};

enum {
    WTK_MOD_SHIFT    = 0x01,
    WTK_MOD_CTRL     = 0x02,
    WTK_MOD_ALT      = 0x04,
    WTK_MOD_SUPER    = 0x08,
    WTK_MOD_CAPSLOCK = 0x10,
};

///////////////////////////////////////////////////////////////////////////////
/// Types

typedef struct wtk_window_t wtk_window_t;

typedef struct wtk_event_t {
    int type;
    int key, button, mods;
    struct { int x, y; } location, delta;
} wtk_event_t;

typedef struct wtk_window_desc_t {
    void (*callback)(wtk_window_t *window, wtk_event_t const *event);
    char const *title;
    int w, h;
} wtk_window_desc_t;

///////////////////////////////////////////////////////////////////////////////
/// Functions

wtk_window_t   *wtk_window_create       (wtk_window_desc_t const *desc);
void            wtk_window_make_current (wtk_window_t *window);
void            wtk_window_swap_buffers (wtk_window_t *window);
void            wtk_window_delete       (wtk_window_t *window);

void            wtk_poll_events         (void);

void            wtk_window_pos          (wtk_window_t const *window, int *x, int *y);
void            wtk_window_size         (wtk_window_t const *window, int *w, int *h);
int             wtk_window_closed       (wtk_window_t const *window);

void            wtk_window_set_pos      (wtk_window_t *window, int x, int y);
void            wtk_window_set_size     (wtk_window_t *window, int w, int h);
void            wtk_window_set_title    (wtk_window_t *window, char const *title);
void            wtk_window_set_closed   (wtk_window_t *window, int closed);

///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                             Implementation                              ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

#if defined(WTK_IMPL)

#include "wtk.h"
#include <stdlib.h> // calloc, free
#include <stdio.h>

#if !defined(WTK_API_WIN32) && !defined(WTK_API_X11) && !defined(WTK_API_COCOA) 
    #if defined(_WIN32)
        #define WTK_API_WIN32
    #elif defined(__linux__)
        #define WTK_API_X11
    #elif defined(__APPLE__) && defined(__OBJC__)
        #define WTK_API_COCOA
    #else
        #error "Unsupported platform"
    #endif
#endif

#if defined(WTK_API_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
    #include <GL/gl.h>
    #include <GL/wgl.h>
    // https://gist.github.com/nickrolfe/1127313ed1dbf80254b614a721b3ee9c
    typedef HGLRC WINAPI _WtkWglCreateContextAttribsARBProc(HDC hdc, HGLRC hShareContext, const int *attribList);
    typedef BOOL WINAPI _WtkWglChoosePixelFormatARBProc(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
    #define WGL_CONTEXT_MAJOR_VERSION_ARB             0x2091
    #define WGL_CONTEXT_MINOR_VERSION_ARB             0x2092
    #define WGL_CONTEXT_PROFILE_MASK_ARB              0x9126
    #define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
    #define WGL_DRAW_TO_WINDOW_ARB                    0x2001
    #define WGL_ACCELERATION_ARB                      0x2003
    #define WGL_SUPPORT_OPENGL_ARB                    0x2010
    #define WGL_DOUBLE_BUFFER_ARB                     0x2011
    #define WGL_PIXEL_TYPE_ARB                        0x2013
    #define WGL_COLOR_BITS_ARB                        0x2014
    #define WGL_DEPTH_BITS_ARB                        0x2022
    #define WGL_STENCIL_BITS_ARB                      0x2023
    #define WGL_FULL_ACCELERATION_ARB                 0x2027
    #define WGL_TYPE_RGBA_ARB                         0x202B
#elif defined(WTK_API_X11)
    #include <X11/Xlib.h>
    #include <X11/keysym.h>
    #include <X11/XKBlib.h>
    #include <GL/glx.h>
    typedef GLXContext _WtkGlXCreateContextAttribsARBProc(Display *, GLXFBConfig, GLXContext, Bool, int const *);
#elif defined(WTK_API_COCOA)
    #import <Cocoa/Cocoa.h>
    @interface _WtkCocoaApp : NSObject <NSApplicationDelegate>
    @end
    @interface _WtkCocoaView : NSOpenGLView <NSWindowDelegate>
    - (id)initWithFrame:(NSRect)frame window:(wtk_window_t *)window;
    @end
#endif

struct wtk_window_t {
    wtk_window_desc_t desc;
    int x, y, closed;
    wtk_window_t *next;
#if defined(WTK_API_WIN32)
    HWND window;
    HDC device;
    HGLRC context;
#elif defined(WTK_API_X11)
    Window window;
    GLXContext context;
#elif defined(WTK_API_COCOA)
    NSWindow *window;
    _WtkCocoaView *view;
#endif
};

static struct {
#if defined(WTK_API_WIN32)
    struct {
        _WtkWglCreateContextAttribsARBProc *wglCreateContextAttribsARB;
        _WtkWglChoosePixelFormatARBProc *wglChoosePixelFormatARB;
    } win32;
#elif defined(WTK_API_X11)
    struct {
        _WtkGlXCreateContextAttribsARBProc *glx_create_ctx_attribs;
        Display *display;
        XContext context;
        Visual *visual;
        Window root;
        Colormap colormap;
        Atom wm_delwin;
        int screen;
        int depth;
    } x11;
#elif defined(WTK_API_COCOA)
    struct {
        _WtkCocoaApp *app;
    } cocoa;
#endif
    wtk_window_t *window_list;
} _wtk = {0};

// Win32 {{{

#if defined(WTK_API_WIN32)

static LRESULT CALLBACK _wtk_window_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    wtk_window_t *window = (wtk_window_t *)GetWindowLongPtr(wnd, GWLP_USERDATA);

    switch (msg) {
        case WM_CLOSE: {
            wtk_window_set_closed(window, 1);
            window->desc.callback(window, &(wtk_event_t){.type = WTK_EVENTTYPE_WINDOWCLOSE});
        } return 0;

        default: {
        } break;
    }

    return DefWindowProc(wnd, msg, wparam, lparam);
}

static int _wtk_init(void) {
    WNDCLASS wnd_class = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = DefWindowProc,
        .hInstance = GetModuleHandle(NULL),
        .lpszClassName = "DummyWindowClass"
    };

    if (!RegisterClass(&wnd_class))
        return 0;

    HWND dummy_wnd = CreateWindow(
        wnd_class.lpszClassName, "Dummy OpenGL Window", 0,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, wnd_class.hInstance, 0
    );

    if (!dummy_wnd)
        return 0;

    HDC dummy_dc = GetDC(dummy_wnd);

    PIXELFORMATDESCRIPTOR pfd = {
        .nsize = sizeof pfd,
        .nVersion = 1,
        .iPixelType = PFD_TYPE_RGBA,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .cColorBits = 32,
        .cAlphaBits = 8,
        .iLayerType = PFD_MAIN_PLANE,
        .cDepthBits = 24,
        .cStencilBits = 8,
    };

    int pixel_format = ChoosePixelFormat(dummy_dc, &pfd);
    if (!pixel_format)
        return 0;

    if (!SetPixelFormat(dummy_dc, pixel_format, &pfd))
        return 0;

    HGLRC dummy_ctx = wglCreateContext(dummy_dc);
    if (!dummy_ctx)
        return 0;

    if (!wglMakeCurrent(dummy_dc, dummy_ctx))
        return 0;

    _wtk.win32.wglCreateContextAttribsARB = (_WtkWglCreateContextAttribsARBProc *)wglGetProcAddress("wglCreateContextAttribsARB");
    _wtk.win32.wglChoosePixelFormatARB = (_WtkWglChoosePixelFormatARBProc *)wglGetProcAddress("wglChoosePixelFormatARB");

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_ctx);
    ReleaseDC(dummy_wnd, dummy_dc);
    DestroyWindow(dummy_wnd);

    return 1;
}

static void _wtk_quit(void) {
    // Move along...
}

static int _wtk_window_create(wtk_window_t *window) {
    WNDCLASS wnd_class = {
        .style = CS_OWNDC,
        .lpfnWndProc = _wtk_window_proc,
        .hInstance = GetModuleHandle(NULL),
        .hCursor = LoadCursor(0, IDC_ARROW),
        .lpszClassName = "wtk_window_tClass",
    };

    if (!RegisterClass(&wnd_class))
        return 0;

    RECT rect = { .right = window->desc.w, .bottom = window->desc.h };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, 0);

    window->window = CreateWindow(
        wnd_class.lpszClassName, window->desc.title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    if (!window->window)
        return 0;

    window->device = GetDC(window->window);

    int pf_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
        WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,         32,
        WGL_DEPTH_BITS_ARB,         24,
        WGL_STENCIL_BITS_ARB,       8,
        0
    };

    int pixel_format;
    UINT num_formats;
    _wtk.win32.wglChoosePixelFormatARB(window->device, pf_attribs, 0, 1, &pixel_format, &num_formats);
    if (!num_formats)
        return 0;

    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(window->device, pixel_format, sizeof pfd, &pfd);
    if (!SetPixelFormat(window->device, pixel_format, &pfd))
        return 0;

    int ctx_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    window->context = _wtk.win32.wglCreateContextAttribsARB(window->device, 0, ctx_attribs);
    if (!window->context)
        return 0;

    ShowWindow(window->window, SW_SHOW);
    return 1;
}

static void _wtk_window_make_current(wtk_window_t *window) {
    wglMakeCurrent(window->device, window->context);
}

static void _wtk_window_swap_buffers(wtk_window_t *window) {
    SwapBuffers(window->device);
}

static void _wtk_poll_events(void) {
    for (MSG msg; PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static void _wtk_window_delete(wtk_window_t *window) {
    ReleaseDC(window->window, window->device);
    DestroyWindow(window->window);
    wglDeleteContext(window->context);
}

static void _wtk_window_set_pos(wtk_window_t *window, int x, int y) {
    SetWindowPos(window->window, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

static void _wtk_window_set_size(wtk_window_t *window, int w, int h) {
    SetWindowPos(window->window, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
}

static void _wtk_window_set_title(wtk_window_t *window, char const *title) {
    SetWindowText(window->window, title);
}

// }}}
// X11 {{{

#elif defined(WTK_API_X11)

static int _wtk_translate_key(int xkey, int state) {
    xkey = XkbKeycodeToKeysym(_wtk.x11.display, xkey, 0, (state & ShiftMask) ? 1 : 0);
    switch(xkey) {
        case XK_BackSpace:  return WTK_KEY_BACKSPACE;
        case XK_Tab:        return WTK_KEY_TAB;
        case XK_Return:     return WTK_KEY_ENTER;
        case XK_Escape:     return WTK_KEY_ESCAPE;
        case XK_Up:         return WTK_KEY_UP;
        case XK_Down:       return WTK_KEY_DOWN;
        case XK_Left:       return WTK_KEY_LEFT;
        case XK_Right:      return WTK_KEY_RIGHT;
        case XK_Page_Up:    return WTK_KEY_PAGEUP;
        case XK_Page_Down:  return WTK_KEY_PAGEDOWN;
        case XK_Home:       return WTK_KEY_HOME;
        case XK_End:        return WTK_KEY_END;
        case XK_Insert:     return WTK_KEY_INSERT;
        case XK_Delete:     return WTK_KEY_DELETE;
        case XK_F1:         return WTK_KEY_F1;
        case XK_F2:         return WTK_KEY_F2;
        case XK_F3:         return WTK_KEY_F3;
        case XK_F4:         return WTK_KEY_F4;
        case XK_F5:         return WTK_KEY_F5;
        case XK_F6:         return WTK_KEY_F6;
        case XK_F7:         return WTK_KEY_F7;
        case XK_F8:         return WTK_KEY_F8;
        case XK_F9:         return WTK_KEY_F9;
        case XK_F10:        return WTK_KEY_F10;
        case XK_F11:        return WTK_KEY_F11;
        case XK_F12:        return WTK_KEY_F12;
        case XK_Shift_L:    return WTK_KEY_LSHIFT;
        case XK_Shift_R:    return WTK_KEY_RSHIFT;
        case XK_Control_L:  return WTK_KEY_LCTRL;
        case XK_Control_R:  return WTK_KEY_RCTRL;
        case XK_Super_L:    return WTK_KEY_LSUPER;
        case XK_Super_R:    return WTK_KEY_RSUPER;
        case XK_Alt_L:      return WTK_KEY_LALT;
        case XK_Alt_R:      return WTK_KEY_RALT;
        case XK_Caps_Lock:  return WTK_KEY_CAPSLOCK;
        default:            return xkey;
    }
}

static unsigned int _wtk_translate_mods(int state) {
    unsigned int mods = 0;

    if (state & ControlMask) mods |= WTK_MOD_CTRL;
    if (state & ShiftMask)   mods |= WTK_MOD_SHIFT;
    if (state & Mod1Mask)    mods |= WTK_MOD_ALT;
    if (state & Mod4Mask)    mods |= WTK_MOD_SUPER;
    if (state & LockMask)    mods |= WTK_MOD_CAPSLOCK;

    return mods;
}

static void _wtk_post_event(wtk_window_t *window, int type, XEvent const *xevent) {
    static wtk_event_t previousEvent = {0};
    wtk_event_t event = {.type = type};
    if (type == WTK_EVENTTYPE_KEYDOWN || type == WTK_EVENTTYPE_KEYUP) {
        event.key        = _wtk_translate_key(xevent->xkey.keycode, xevent->xkey.state);
        event.mods       = xevent->xkey.state;
        event.location.x = xevent->xkey.x;
        event.location.y = xevent->xkey.y;
    } else if (type == WTK_EVENTTYPE_MOUSEDOWN || type == WTK_EVENTTYPE_MOUSEUP) {
        switch (xevent->xbutton.button) {
            case Button4: event.delta.y =  1.0; break;
            case Button5: event.delta.y = -1.0; break;
            case 6:       event.delta.x =  1.0; break;
            case 7:       event.delta.x = -1.0; break;
            default:      event.button  = xevent->xbutton.button - Button1 - 4; break;
        }
        event.mods       = xevent->xbutton.state;
        event.location.x = xevent->xbutton.x;
        event.location.y = xevent->xbutton.y;
    } else if (type == WTK_EVENTTYPE_MOUSEMOTION) {
        event.mods       = xevent->xmotion.state;
        event.location.x = xevent->xmotion.x;
        event.location.y = xevent->xmotion.y;
    }

    event.mods = _wtk_translate_mods(event.mods);
    event.delta.x = event.location.x - previousEvent.location.x;
    event.delta.y = event.location.y - previousEvent.location.y;

    window->desc.callback(window, &event);
    previousEvent = event;
}

int _wtk_init(void) {
    if (!(_wtk.x11.display = XOpenDisplay(NULL)))
        return 0;

    _wtk.x11.context = XUniqueContext();
    _wtk.x11.screen  = DefaultScreen(_wtk.x11.display);
    _wtk.x11.root    = RootWindow(_wtk.x11.display, _wtk.x11.screen);
    _wtk.x11.visual  = DefaultVisual(_wtk.x11.display, _wtk.x11.screen);
    _wtk.x11.depth   = DefaultDepth(_wtk.x11.display, _wtk.x11.screen);

    if (!(_wtk.x11.colormap = XCreateColormap(_wtk.x11.display, _wtk.x11.root, _wtk.x11.visual, AllocNone)))
        return 0;

    if (!(_wtk.x11.wm_delwin = XInternAtom(_wtk.x11.display, "WM_DELETE_WINDOW", 0)))
        return 0;

    _wtk.x11.glx_create_ctx_attribs = (_WtkGlXCreateContextAttribsARBProc *)glXGetProcAddressARB((GLubyte const *)"glXCreateContextAttribsARB");

    return 1;
}

void _wtk_quit(void) {
    XFreeColormap(_wtk.x11.display, _wtk.x11.colormap);
    XCloseDisplay(_wtk.x11.display);
}

int _wtk_window_create(wtk_window_t *window) {
    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = _wtk.x11.colormap
    };

    window->window = XCreateWindow(
        _wtk.x11.display, _wtk.x11.root,
        window->x, window->y, window->desc.w, window->desc.h,
        0, _wtk.x11.depth, InputOutput,
        _wtk.x11.visual, CWColormap | CWEventMask, &swa
    );
    if (!window->window) return 0;

    if (!XSetWMProtocols(_wtk.x11.display, window->window, &_wtk.x11.wm_delwin, 1))
        return 0;

    GLint vis_attribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(_wtk.x11.display, _wtk.x11.screen, vis_attribs, &fbcount);
    if (!fbc || !fbcount) return 0;

    GLint ctx_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    if (_wtk.x11.glx_create_ctx_attribs)
        window->context = _wtk.x11.glx_create_ctx_attribs(_wtk.x11.display, fbc[0], NULL, 1, ctx_attribs);
    else
        window->context = glXCreateNewContext(_wtk.x11.display, fbc[0], GLX_RGBA_TYPE, NULL, 1);

    XSaveContext(_wtk.x11.display, window->window, _wtk.x11.context, (XPointer)window);
    XMapWindow(_wtk.x11.display, window->window);
    XFlush(_wtk.x11.display);
    return 1;
}

void _wtk_window_make_current(wtk_window_t *window) {
    glXMakeContextCurrent(_wtk.x11.display, window->window, window->window, window->context);
}

void _wtk_window_swap_buffers(wtk_window_t *window) {
    glXSwapBuffers(_wtk.x11.display, window->window);
}

void _wtk_poll_events(void) {
    wtk_window_t *window;
    XEvent event;

    while (XPending(_wtk.x11.display)) {
        XNextEvent(_wtk.x11.display, &event);
        if (XFindContext(_wtk.x11.display, event.xany.window, _wtk.x11.context, (XPointer *)&window))
            continue;

        switch (event.type) {
            case KeyPress:      _wtk_post_event(window, WTK_EVENTTYPE_KEYDOWN, &event);        break;
            case KeyRelease:    _wtk_post_event(window, WTK_EVENTTYPE_KEYUP, &event);          break;
            case ButtonPress:   _wtk_post_event(window, WTK_EVENTTYPE_MOUSEDOWN, &event);      break;
            case ButtonRelease: _wtk_post_event(window, WTK_EVENTTYPE_MOUSEUP, &event);        break;
            case MotionNotify:  _wtk_post_event(window, WTK_EVENTTYPE_MOUSEMOTION, &event);    break;
            case EnterNotify:   _wtk_post_event(window, WTK_EVENTTYPE_MOUSEENTER, &event);     break;
            case LeaveNotify:   _wtk_post_event(window, WTK_EVENTTYPE_MOUSELEAVE, &event);     break;
            case MapNotify:     _wtk_post_event(window, WTK_EVENTTYPE_WINDOWFOCUSIN, &event);  break;
            case UnmapNotify:   _wtk_post_event(window, WTK_EVENTTYPE_WINDOWFOCUSOUT, &event); break;
            case ConfigureNotify: {
                window->x = event.xconfigure.x;
                window->y = event.xconfigure.y;
                window->desc.w = event.xconfigure.width;
                window->desc.h = event.xconfigure.height;
                _wtk_post_event(window, WTK_EVENTTYPE_WINDOWRESIZE, &event);
            } break;
            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == _wtk.x11.wm_delwin) {
                    wtk_window_set_closed(window, 1);
                    _wtk_post_event(window, WTK_EVENTTYPE_WINDOWCLOSE, &event);
                }
            } break;
        }
    }
}

void _wtk_window_delete(wtk_window_t *window) {
    glXDestroyContext(_wtk.x11.display, window->context);
    XDestroyWindow(_wtk.x11.display, window->window);
}

void _wtk_window_set_pos(wtk_window_t *window, int x, int y) {
    XMoveWindow(_wtk.x11.display, window->window, x, y);
}

void _wtk_window_set_size(wtk_window_t *window, int w, int h) {
    XResizeWindow(_wtk.x11.display, window->window, (unsigned int)w, (unsigned int)h);
}

void _wtk_window_set_title(wtk_window_t *window, char const *title) {
    XStoreName(_wtk.x11.display, window->window, title);
}

// }}}
// Cocoa {{{

#elif defined(WTK_API_COCOA)

static void _wtk_post_event(wtk_window_t *window, int type, NSEvent *event) {
    wtk_event_t ev = {
        .type       = type,
        .mods       = (int)[event modifierFlags],
        .location   = {(int)[event locationInWindow].x, (int)[event locationInWindow].y},
        .delta      = {(int)[event deltaX], (int)[event deltaY]}
    };

    if (type == WTK_EVENTTYPE_KEYDOWN || type == WTK_EVENTTYPE_KEYUP)
        ev.key = [event keyCode];
    else if (type == WTK_EVENTTYPE_MOUSEDOWN || type == WTK_EVENTTYPE_MOUSEUP)
        ev.button = [event buttonNumber];

    window->desc.callback(window, &ev);
}

static float _wtk_flip_y(float y) {
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}

@implementation _WtkCocoaApp
- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    id appMenu = [[NSMenu new] autorelease];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];

    [menubar addItem:appMenuItem];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
    [NSApp setMainMenu:menubar];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    for (wtk_window_t *window = _wtk.window_list; window; window = window->next) {
        wtk_window_set_closed(window, 1);
        window->desc.callback(window, &(wtk_event_t){.type = WTK_EVENTTYPE_WINDOWCLOSE});
    }

    return NSTerminateCancel;
}
@end

@implementation _WtkCocoaView {
    wtk_window_t *m_window;
}

- (id)initWithFrame:(NSRect)frame window:(wtk_window_t *)window  {
    NSOpenGLPixelFormatAttribute attributes[] = {
        NSOpenGLPFAOpenGLProfile,   NSOpenGLProfileVersion4_1Core,
        NSOpenGLPFAMultisample,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize,       32,
        NSOpenGLPFADepthSize,       24,
        NSOpenGLPFAAlphaSize,        8,
        0
    };

    if (self = [super initWithFrame:frame pixelFormat:[[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease]])
        m_window = window;
    return self;
}

- (BOOL)windowShouldClose:(NSNotification *)notification {
    wtk_window_set_closed(m_window, 1);
    _wtk_post_event(m_window, WTK_EVENTTYPE_WINDOWCLOSE, NULL);
    return NO;
}

- (void)keyDown:(NSEvent *)event         { _wtk_post_event(m_window, WTK_EVENTTYPE_KEYDOWN, event); }
- (void)keyUp:(NSEvent *)event           { _wtk_post_event(m_window, WTK_EVENTTYPE_KEYUP, event); }
- (void)mouseDown:(NSEvent *)event       { _wtk_post_event(m_window, WTK_EVENTTYPE_MOUSEDOWN, event); }
- (void)mouseUp:(NSEvent *)event         { _wtk_post_event(m_window, WTK_EVENTTYPE_MOUSEUP, event);   }
- (void)rightMouseDown:(NSEvent *)event  { [self mouseDown:event]; }
- (void)rightMouseUp:(NSEvent *)event    { [self mouseUp:event];   }
- (void)otherMouseDown:(NSEvent *)event  { [self mouseDown:event]; }
- (void)otherMouseUp:(NSEvent *)event    { [self mouseUp:event];   }
- (void)mouseEntered:(NSEvent *)event    { _wtk_post_event(m_window, WTK_EVENTTYPE_MOUSEENTER, event); }
- (void)mouseExited:(NSEvent *)event     { _wtk_post_event(m_window, WTK_EVENTTYPE_MOUSELEAVE, event); }
- (void)mouseMoved:(NSEvent *)event      { _wtk_post_event(m_window, WTK_EVENTTYPE_MOUSEMOTION, event); }
- (void)scrollWheel:(NSEvent *)event     { _wtk_post_event(m_window, WTK_EVENTTYPE_MOUSESCROLL, event); }

@end

int _wtk_init(void) {
    @autoreleasepool {

    [NSApplication sharedApplication];
    _wtk.cocoa.app = [[_WtkCocoaApp alloc] init];

    [NSApp setDelegate:_wtk.cocoa.app];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp finishLaunching];

    return 1;

    }
}

void _wtk_quit(void) {
    @autoreleasepool {

    [NSApp terminate:nil];
    [_wtk.cocoa.app release];

    }
}

int _wtk_window_create(wtk_window_t *window) {
    @autoreleasepool {

    unsigned styleMask = NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable;
    NSRect frame = NSMakeRect(0, 0, window->desc.w, window->desc.h);

    window->window = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
    if (!window->window) return 0;

    window->view = [[_WtkCocoaView alloc] initWithFrame:frame window:window];
    if (!window->view) return 0;

    [window->window setContentView:window->view];
    [window->window setDelegate:window->view];
    [window->window makeFirstResponder:window->view];
    [window->window setAcceptsMouseMovedEvents:YES];
    [window->window setRestorable:NO];
    [window->window makeKeyAndOrderFront:nil];
    [window->window center];

    return 1;

    }
}

void _wtk_window_make_current(wtk_window_t *window) {
    @autoreleasepool {

    [[window->view openGLContext] makeCurrentContext];

    }
}

void _wtk_window_swap_buffers(wtk_window_t *window) {
    @autoreleasepool {

    [[window->view openGLContext] flushBuffer];

    }
}

void _wtk_poll_events(void) {
    @autoreleasepool {

    for (NSEvent *event; (event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];))
        [NSApp sendEvent:event];

    }
}

void _wtk_window_delete(wtk_window_t *window) {
    @autoreleasepool {

    if (window->window) {
        if (window->view)
            [window->view release];
        [window->window setDelegate:nil];
        [window->window close];
    }

    }
}

void _wtk_window_set_pos(wtk_window_t *window, int x, int y) {
    @autoreleasepool {

    NSRect rect  = NSMakeRect(x, _wtk_flip_y(y + [window->view frame].size.height - 1), 0, 0);
    NSRect frame = [window->window frameRectForContentRect:rect];
    [window->window setFrameOrigin:frame.origin];

    }
}

void _wtk_window_set_size(wtk_window_t *window, int w, int h) {
    @autoreleasepool {

    NSRect contentRect = [window->window contentRectForFrameRect:[window->window frame]];
    contentRect.origin.y += contentRect.size.height - h;
    contentRect.size = NSMakeSize(w, h);
    [window->window setFrame:[window->window frameRectForContentRect:contentRect] display:YES];

    }
}

void _wtk_window_set_title(wtk_window_t *window, char const *title) {
    @autoreleasepool {

    [window->window setTitle:@(title)];

    }
}

#endif // WTK_API_WIN32 || WTK_API_X11 || WTK_API_COCOA

// }}}
// Common {{{

static void _wtk_default_event_callback(wtk_window_t *window, wtk_event_t const *event) {
    (void)window; (void)event;
}

static void _wtk_validate_desc(wtk_window_t *window) {
    if (!window->desc.callback) window->desc.callback = _wtk_default_event_callback;
    if (!window->desc.title)    window->desc.title = "";
    if (!window->desc.w)        window->desc.w = 640;
    if (!window->desc.h)        window->desc.h = 480;
}

wtk_window_t *wtk_window_create(wtk_window_desc_t const *desc) {
    if (!desc)
        return NULL;

    if (!_wtk.window_list)
        if (!_wtk_init())
            return NULL;

    wtk_window_t *window = calloc(1, sizeof *window);
    if (!window) return NULL;

    window->desc = *desc;
    _wtk_validate_desc(window);

    if (!_wtk_window_create(window)) {
        wtk_window_delete(window);
        return NULL;
    }

    window->next = _wtk.window_list;
    _wtk.window_list = window;

    return window;
}

void wtk_window_make_current(wtk_window_t *window) {
    if (window)
        _wtk_window_make_current(window);
}

void wtk_window_swap_buffers(wtk_window_t *window) {
    if (window)
        _wtk_window_swap_buffers(window);
}

void wtk_poll_events(void) {
    _wtk_poll_events();
}

void wtk_window_delete(wtk_window_t *window) {
    if (!window) return;

    _wtk_window_delete(window);
    free(window);

    wtk_window_t **prev = &_wtk.window_list;
    while (*prev != window)
        prev = &((*prev)->next);
    *prev = window->next;

    if (!_wtk.window_list)
        _wtk_quit();
}

void wtk_window_pos(wtk_window_t const *window, int *x, int *y) {
    if (x) *x = window ? window->x : -1;
    if (y) *y = window ? window->y : -1;
}

void wtk_window_size(wtk_window_t const *window, int *w, int *h) {
    if (w) *w = window ? window->desc.w : -1;
    if (h) *h = window ? window->desc.h : -1;
}

int wtk_window_closed(wtk_window_t const *window) {
    return window ? window->closed : 1;
}

void wtk_window_set_pos(wtk_window_t *window, int x, int y) {
    if (!window || x < 0 || y < 0) return;

    _wtk_window_set_pos(window, x, y);
    window->x = x;
    window->y = y;
}

void wtk_window_set_size(wtk_window_t *window, int w, int h) {
    if (!window || w < 0 || h < 0) return;

    _wtk_window_set_size(window, w, h);
    window->desc.w = w;
    window->desc.h = h;
}

void wtk_window_set_title(wtk_window_t *window, char const *title) {
    if (!window || !title) return;

    _wtk_window_set_title(window, title);
    window->desc.title = title;
}

void wtk_window_set_closed(wtk_window_t *window, int closed) {
    if (window)
        window->closed = closed;
}

// }}}

#endif // WTK_IMPL
