#pragma once
#include "wtk/wtk.h"

#if defined(WTK_USE_COCOA)

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

#elif defined(WTK_USE_X11)

    #include <X11/Xlib.h>
    #include "plugins/glad/glx.h"

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

#if defined(WTK_USE_COCOA)
    CocoaPlatform       cocoa;
#elif defined(WTK_USE_X11)
    X11Platform         x11;
#endif
} Wtk;

struct WtkWindow {
    WtkEventCallback   *onEvent;
    char const         *title;
    int                 x, y, w, h;
    int                 shouldClose;

#if defined(WTK_USE_COCOA)
    CocoaWindow         cocoa;
#elif defined(WTK_USE_X11)
    X11Window           x11;
#endif
};

extern Wtk WTK;
