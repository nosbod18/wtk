#pragma once
#include "wtk/wtk.h"
#include <stdbool.h>

#if defined(WTK_USE_COCOA) && defined(__OBJC__)

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
        CFBundleRef     bundle;
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

#if defined(WTK_USE_COCOA) && defined(__OBJC__)
    CocoaPlatform       cocoa;
#elif defined(WTK_USE_X11)
    X11Platform         x11;
#endif
} Wtk;

struct WtkWindow {
    WtkEventCallback   *onEvent;
    char const         *title;
    int                 x, y, w, h;
    bool                shouldClose;

#if defined(WTK_USE_COCOA) && defined(__OBJC__)
    CocoaWindow         cocoa;
#elif defined(WTK_USE_X11)
    X11Window           x11;
#endif
};

extern Wtk WTK;

bool            platformStart           (void);
void            platformStop            (void);

bool            platformCreateWindow    (WtkWindow *window);
void            platformDeleteWindow    (WtkWindow *window);
bool            platformCreateContext   (WtkWindow *window, WtkWindowDesc const *desc);
void            platformDeleteContext   (WtkWindow *window);

void            platformMakeCurrent     (WtkWindow const *window);
void            platformSwapBuffers     (WtkWindow const *window);
void            platformPollEvents      (void);

void            platformSetWindowPos    (WtkWindow *window, int x, int y);
void            platformSetWindowSize   (WtkWindow *window, int w, int h);
void            platformSetWindowTitle  (WtkWindow *window, char const *title);

WtkGLProc      *platformGetProcAddress   (char const *name);