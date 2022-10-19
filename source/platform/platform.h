#pragma once
#include "wtk/wtk.h"

#if defined(__APPLE__)

    #include <Cocoa/Cocoa.h>

    @interface AppDelegate : NSObject <NSApplicationDelegate>
    @end

    @interface ContentView : NSOpenGLView <NSWindowDelegate> {
        WtkWindow *m_window;
    }
    @end

    typedef struct CocoaWindow {
        NSWindow       *window;
        ContentView    *view;
    } CocoaWindow;

    typedef struct CocoaPlatform {
        AppDelegate    *appDelegate;
    } CocoaPlatform;

#elif defined(__linux__)

    #include <X11/Xlib.h>
    #include "plugins/glad/glx.h"

    typedef GLXContext GlXCreateContextAttribsARB(Display *, GLXFBConfig, GLXContext, Bool, int const *);

    typedef struct X11Window {
        Window          window;
        GLXContext      context;
    } X11Window;

    typedef struct X11Platform {
        Display        *display;
        XContext        context;
        Visual         *visual;
        Window          root;
        Colormap        colormap;
        Atom            wmDeleteWindow;
        int             screen;
        int             depth;
    } X11Platform;
#endif

typedef struct Wtk {
    int                 windowCount;

#if defined(__APPLE__)
    CocoaPlatform       cocoa;
#elif defined(__linux__)
    X11Platform         x11;
#endif
} Wtk;

struct WtkWindow {
    WtkEventCallback   *onEvent;
    char const         *title;
    int                 x, y, w, h;
    int                 shouldClose;

#if defined(__APPLE__)
    CocoaWindow         cocoa;
#elif defined(__linux__)
    X11Window           x11;
#endif
};

extern Wtk WTK;
