#pragma once
#include "wnd/wnd.h"
#include <stdbool.h>

#if defined(__APPLE__) && defined(__OBJC__)

    #include <Cocoa/Cocoa.h>

    @interface AppDelegate : NSObject <NSApplicationDelegate>
    @end

    @interface ContentView : NSOpenGLView <NSWindowDelegate> {
        WndWindow *m_window;
    }
    @end

    typedef struct CocoaWindow {
        NSWindow *window;
        ContentView *view;
    } CocoaWindow;

    typedef struct CocoaPlatform {
        AppDelegate *appDelegate;
        CFBundleRef bundle;
    } CocoaPlatform;

#elif defined(__linux__)

    #include <X11/Xlib.h>
    #include <GL/glx.h>

    typedef GLXContext glXCreateContextAttribsARBProc(Display *, GLXFBConfig, GLXContext, Bool, int const *);

    typedef struct X11Window {
        Window window;
        GLXContext context;
    } X11Window;

    typedef struct X11Platform {
        glXCreateContextAttribsARBProc *glXCreateContextAttribsARB;
        Display *display;
        XContext context;
        Visual *visual;
        Window root;
        Colormap colormap;
        Atom wmDeleteWindow;
        int screen;
        int depth;
    } X11Platform;
#endif

typedef struct Wnd {
    int windowCount;

#if defined(__APPLE__) && defined(__OBJC__)
    CocoaPlatform cocoa;
#elif defined(__linux__)
    X11Platform x11;
#endif
} Wnd;

struct WndWindow {
    WndEventCallback *onEvent;
    char const *title;
    int x, y, w, h;
    bool shouldClose;

#if defined(__APPLE__) && defined(__OBJC__)
    CocoaWindow cocoa;
#elif defined(__linux__)
    X11Window x11;
#endif
};

extern Wnd _wnd;

bool        platformStart           (void);
void        platformStop            (void);

bool        platformCreateWindow    (WndWindow *window);
void        platformDeleteWindow    (WndWindow *window);
bool        platformCreateContext   (WndWindow *window, WndWindowDesc const *desc);
void        platformDeleteContext   (WndWindow *window);

void        platformMakeCurrent     (WndWindow const *window);
void        platformSwapBuffers     (WndWindow const *window);
void        platformPollEvents      (void);

void        platformSetWindowPos    (WndWindow *window, int x, int y);
void        platformSetWindowSize   (WndWindow *window, int w, int h);
void        platformSetWindowTitle  (WndWindow *window, char const *title);

WndGLProc  *platformGetProcAddress   (char const *name);
