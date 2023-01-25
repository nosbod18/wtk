#include "wtk.h"
#include <stdlib.h> // calloc, free

#if defined(__APPLE__) && defined(__OBJC__) && !defined(WTK_COCOA)
    #define WTK_COCOA
#elif defined(__linux__) && !defined(WTK_X11)
    #define WTK_X11
#else
    #error "Unsupported platform"
#endif

#if defined(WTK_X11)
    #include <X11/Xlib.h>
    #include <GL/glx.h>

    typedef GLXContext glXCreateContextAttribsARBProc(Display *, GLXFBConfig, GLXContext, Bool, int const *);
#elif defined(WTK_COCOA)
    #import <Cocoa/Cocoa.h>

    @interface CocoaApp : NSObject <NSApplicationDelegate>
    @end

    @interface CocoaView : NSOpenGLView <NSWindowDelegate>
    - (id)initWithFrame:(NSRect)frame andWindow:(WtkWindow *)window;
    @end
#endif

struct WtkWindow {
    WtkWindowDesc desc;
    bool closed;
#if defined(WTK_X11)
    Window window;
    GLXContext context;
#elif defined(WTK_COCOA)
    NSWindow *window;
    CocoaView *view;
#endif
};

static struct {
#if defined(WTK_X11)
    struct {
        glXCreateContextAttribsARBProc *glx_create_ctx_attribs;
        Display *display;
        XContext context;
        Visual *visual;
        Window root;
        Colormap colormap;
        Atom wm_delwin;
        int screen;
        int depth;
    } x11;
#elif defined(WTK_COCOA)
    struct {
        CocoaApp *app;
    } cocoa;
#endif
    int num_windows;
} G = {0};

///
/// X11
///

#if defined(WTK_X11)

static void postEvent(WtkWindow *window, WtkEventType type, XEvent const *event) {
    WtkEvent ev = {0};
    if (type == WtkEventType_KeyDown || type == WtkEventType_KeyUp) {
        ev.key.code = event->xkey.keycode;
        ev.key.sym  = event->xkey.keycode;
        ev.key.mods = event->xkey.state;
        ev.key.y    = event->xkey.x;
        ev.key.x    = event->xkey.y;
    } else if (type == WtkEventType_MouseDown || type == WtkEventType_MouseUp){
        switch (event->xbutton.button) {
            case Button4: ev.scroll.dy =  1.0; break;
            case Button5: ev.scroll.dy = -1.0; break;
            case 6:       ev.scroll.dx =  1.0; break;
            case 7:       ev.scroll.dx = -1.0; break;
            default:
                ev.button.code = event->xbutton.button - Button1 - 4;
                ev.button.sym  = event->xbutton.button - Button1 - 4;
                break;
        }
    } else if (type == WtkEventType_MouseMotion) {
        ev.motion.dx = event->xmotion.x;
        ev.motion.dy = event->xmotion.y;
    }
    window->desc.event_handler(window, type, &ev);
}

bool init(void) {
    if (!(G.x11.display = XOpenDisplay(NULL)))
        return false;

    G.x11.context = XUniqueContext();
    G.x11.screen  = DefaultScreen(G.x11.display);
    G.x11.root    = RootWindow(G.x11.display, G.x11.screen);
    G.x11.visual  = DefaultVisual(G.x11.display, G.x11.screen);
    G.x11.depth   = DefaultDepth(G.x11.display, G.x11.screen);

    if (!(G.x11.colormap = XCreateColormap(G.x11.display, G.x11.root, G.x11.visual, AllocNone)))
        return false;

    if (!(G.x11.wm_delwin = XInternAtom(G.x11.display, "WM_DELETE_WINDOW", 0)))
        return false;

    G.x11.glx_create_ctx_attribs = (glXCreateContextAttribsARBProc *)glXGetProcAddressARB((GLubyte const *)"glXCreateContextAttribsARB");

    return true;
}

void quit(void) {
    XFreeColormap(G.x11.display, G.x11.colormap);
    XCloseDisplay(G.x11.display);
}

bool createWindow(WtkWindow *window) {
    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = G.x11.colormap
    };

    window->window = XCreateWindow(
        G.x11.display, G.x11.root,
        window->desc.x, window->desc.y, window->desc.w, window->desc.h,
        0, G.x11.depth, InputOutput,
        G.x11.visual, CWColormap | CWEventMask, &swa
    );
    if (!window->window) return false;

    if (!XSetWMProtocols(G.x11.display, window->window, &G.x11.wm_delwin, 1))
        return false;

    GLint vis_attribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(G.x11.display, G.x11.screen, vis_attribs, &fbcount);
    if (!fbc || !fbcount) return false;

    GLint ctx_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    if (G.x11.glx_create_ctx_attribs)
        window->context = G.x11.glx_create_ctx_attribs(G.x11.display, fbc[0], NULL, 1, ctx_attribs);
    else
        window->context = glXCreateNewContext(G.x11.display, fbc[0], GLX_RGBA_TYPE, NULL, 1);

    XSaveContext(G.x11.display, window->window, G.x11.context, (XPointer)window);
    XMapWindow(G.x11.display, window->window);
    XFlush(G.x11.display);
    return true;
}

void makeCurrent(WtkWindow *window) {
    glXMakeContextCurrent(G.x11.display, window->window, window->window, window->context);
}

void swapBuffers(WtkWindow *window) {
    glXSwapBuffers(G.x11.display, window->window);
}

void pollEvents(void) {
    WtkWindow *window;
    XEvent event;

    while (XPending(G.x11.display)) {
        XNextEvent(G.x11.display, &event);
        if (XFindContext(G.x11.display, event.xany.window, G.x11.context, (XPointer *)&window))
            continue;

        switch (event.type) {
            case KeyPress:      postEvent(window, WtkEventType_KeyDown, &event);        break;
            case KeyRelease:    postEvent(window, WtkEventType_KeyUp, &event);          break;
            case ButtonPress:   postEvent(window, WtkEventType_MouseDown, &event);      break;
            case ButtonRelease: postEvent(window, WtkEventType_MouseUp, &event);        break;
            case MotionNotify:  postEvent(window, WtkEventType_MouseMotion, &event);    break;
            case EnterNotify:   postEvent(window, WtkEventType_MouseEnter, &event);     break;
            case LeaveNotify:   postEvent(window, WtkEventType_MouseLeave, &event);     break;
            case MapNotify:     postEvent(window, WtkEventType_WindowFocusIn, &event);  break;
            case UnmapNotify:   postEvent(window, WtkEventType_WindowFocusOut, &event); break;
            case ConfigureNotify: {
                window->desc.x = event.xconfigure.x;
                window->desc.y = event.xconfigure.y;
                window->desc.w = event.xconfigure.width;
                window->desc.h = event.xconfigure.height;
                postEvent(window, WtkEventType_WindowResize, &event);
            } break;
            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == G.x11.wm_delwin) {
                    WtkSetWindowShouldClose(window, true);
                    postEvent(window, WtkEventType_WindowClose, &event);
                }
            } break;
        }
    }
}

void deleteWindow(WtkWindow *window) {
    glXDestroyContext(G.x11.display, window->context);
    XDestroyWindow(G.x11.display, window->window);
}

void setWindowOrigin(WtkWindow *window, int x, int y) {
    XMoveWindow(G.x11.display, window->window, x, y);
}

void setWindowSize(WtkWindow *window, int w, int h) {
    XResizeWindow(G.x11.display, window->window, (unsigned int)w, (unsigned int)h);
}

void setWindowTitle(WtkWindow *window, char const *title) {
    XStoreName(G.x11.display, window->window, title);
}

///
/// Cocoa
///

#elif defined(WTK_COCOA)

static void postEvent(WtkWindow *window, WtkEventType type, NSEvent *event) {
    WtkEvent ev = {0};
    if (type == WtkEventType_KeyDown || type == WtkEventType_KeyUp) {
        ev.key.code = [event keyCode];
        ev.key.sym  = [event keyCode];
        ev.key.mods = [event modifierFlags];
        ev.key.x    = [event locationInWindow].x;
        ev.key.y    = [event locationInWindow].y;
    } else if (type == WtkEventType_MouseDown || type == WtkEventType_MouseUp) {
        ev.button.code = [event buttonNumber];
        ev.button.sym  = [event buttonNumber];
        ev.button.mods = [event modifierFlags];
        ev.button.x    = [event locationInWindow].x;
        ev.button.y    = [event locationInWindow].y;
    } else if (type == WtkEventType_MouseScroll) {
        ev.motion.dx = [event deltaX];
        ev.motion.dy = [event deltaY];
    }
    window->desc.event_handler(window, type, &ev);
}

static float translateYCoordinate(float y) {
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}

@implementation CocoaApp
-(void)applicationWillFinishLaunching:(NSNotification *)notification {
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
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}
@end

@implementation CocoaView {
    WtkWindow *m_window;
}

- (id)initWithFrame:(NSRect)frame andWindow:(WtkWindow *)window  {
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

-(BOOL)windowShouldClose:(NSNotification *)notification {
    WtkSetWindowShouldClose(m_window, true);
    postEvent(m_window, WtkEventType_WindowClose, NULL);
    return NO;
}

-(void)keyDown:(NSEvent *)event         { postEvent(m_window, WtkEventType_KeyDown, event); }
-(void)keyUp:(NSEvent *)event           { postEvent(m_window, WtkEventType_KeyUp, event); }
-(void)mouseDown:(NSEvent *)event       { postEvent(m_window, WtkEventType_MouseDown, event); }
-(void)mouseUp:(NSEvent *)event         { postEvent(m_window, WtkEventType_MouseUp, event);   }
-(void)rightMouseDown:(NSEvent *)event  { [self mouseDown:event]; }
-(void)rightMouseUp:(NSEvent *)event    { [self mouseUp:event];   }
-(void)otherMouseDown:(NSEvent *)event  { [self mouseDown:event]; }
-(void)otherMouseUp:(NSEvent *)event    { [self mouseUp:event];   }
-(void)mouseEntered:(NSEvent *)event    { postEvent(m_window, WtkEventType_MouseEnter, event); }
-(void)mouseExited:(NSEvent *)event     { postEvent(m_window, WtkEventType_MouseLeave, event); }
-(void)mouseMoved:(NSEvent *)event      { postEvent(m_window, WtkEventType_MouseMotion, event); }
-(void)scrollWheel:(NSEvent *)event     { postEvent(m_window, WtkEventType_MouseScroll, event); }

@end

bool init(void) {
    @autoreleasepool {

    [NSApplication sharedApplication];
    G.cocoa.app = [[CocoaApp alloc] init];

    [NSApp setDelegate:G.cocoa.app];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp finishLaunching];

    return true;

    }
}

void quit(void) {
    @autoreleasepool {

    [NSApp terminate:nil];
    [G.cocoa.app release];

    }
}

bool createWindow(WtkWindow *window) {
    @autoreleasepool {

    unsigned styleMask = NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable;
    NSRect frame = NSMakeRect(0, 0, window->desc.w, window->desc.h);

    window->window = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
    if (!window->window) return false;

    window->view = [[CocoaView alloc] initWithFrame:frame andWindow:window];
    if (!window->view) return false;

    [window->window setContentView:window->view];
    [window->window setDelegate:window->view];
    [window->window makeFirstResponder:window->view];

    [window->window setAcceptsMouseMovedEvents:YES];
    [window->window setRestorable:NO];
    [window->window center];
    [window->window makeKeyAndOrderFront:nil];
    [window->window orderFront:nil];

    return true;

    }
}

void makeCurrent(WtkWindow *window) {
    @autoreleasepool {

    [[window->view openGLContext] makeCurrentContext];

    }
}

void swapBuffers(WtkWindow *window) {
    @autoreleasepool {

    [[window->view openGLContext] flushBuffer];

    }
}

void pollEvents(void) {
    @autoreleasepool {

    while (true) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (!event) break;

        [NSApp sendEvent:event];
    }

    }
}

void deleteWindow(WtkWindow *window) {
    @autoreleasepool {

    if (window->window) {
        if (window->view)
            [window->view release];
        [window->window setDelegate:nil];
        [window->window close];
    }

    }
}

void setWindowOrigin(WtkWindow *window, int x, int y) {
    @autoreleasepool {

    NSRect rect  = NSMakeRect(x, translateYCoordinate(y + [window->view frame].size.height - 1), 0, 0);
    NSRect frame = [window->window frameRectForContentRect:rect];
    [window->window setFrameOrigin:frame.origin];

    }
}

void setWindowSize(WtkWindow *window, int w, int h) {
    @autoreleasepool {

    NSRect contentRect = [window->window contentRectForFrameRect:[window->window frame]];
    contentRect.origin.y += contentRect.size.height - h;
    contentRect.size = NSMakeSize(w, h);
    [window->window setFrame:[window->window frameRectForContentRect:contentRect] display:YES];

    }
}

void setWindowTitle(WtkWindow *window, char const *title) {
    @autoreleasepool {

    [window->window setTitle:@(title)];

    }
}

#endif // WTK_X11 || WTK_COCOA

///
/// Common
///

static void defaultEventCallback(WtkWindow *window, WtkEventType type, WtkEvent const *event) {
    (void)window; (void)type; (void)event;
}

static void validateDesc(WtkWindow *window) {
    WtkWindowDesc *d = &window->desc;
    if (!d->event_handler)  d->event_handler = defaultEventCallback;
    if (!d->title)          d->title = "";
    if (!d->w)              d->w = 640;
    if (!d->h)              d->h = 480;
}

WtkWindow *WtkCreateWindow(WtkWindowDesc const *desc) {
    if (!desc || (G.num_windows == 0 && !init()))
        return NULL;

    WtkWindow *window = calloc(1, sizeof *window);
    if (!window) return NULL;

    window->desc = *desc;
    validateDesc(window);

    if (!createWindow(window)) {
        WtkDeleteWindow(window);
        return NULL;
    }

    G.num_windows++;
    return window;
}

void WtkMakeCurrent(WtkWindow *window) {
    if (window) makeCurrent(window);
}

void WtkSwapBuffers(WtkWindow *window) {
    if (window) swapBuffers(window);
}

void WtkPollEvents(void) {
    pollEvents();
}

void WtkDeleteWindow(WtkWindow *window) {
    if (!window) return;

    deleteWindow(window);
    free(window);

    if (--G.num_windows == 0)
        quit();
}

void WtkGetWindowRect(WtkWindow const *window, int *x, int *y, int *w, int *h) {
    if (x) *x = window ? window->desc.x : -1;
    if (y) *y = window ? window->desc.y : -1;
    if (w) *w = window ? window->desc.w : -1;
    if (h) *h = window ? window->desc.h : -1;
}

bool WtkGetWindowShouldClose(WtkWindow const *window) {
    return window ? window->closed : true;
}

void WtkSetWindowOrigin(WtkWindow *window, int x, int y) {
    if (!window || x < 0 || y < 0) return;

    setWindowOrigin(window, x, y);
    window->desc.x = x;
    window->desc.y = y;
}

void WtkSetWindowSize(WtkWindow *window, int w, int h) {
    if (!window || w < 0 || h < 0) return;

    setWindowSize(window, w, h);
    window->desc.w = w;
    window->desc.h = h;
}

void WtkSetWindowTitle(WtkWindow *window, char const *title) {
    if (!window || !title) return;

    setWindowTitle(window, title);
    window->desc.title = title;
}

void WtkSetWindowShouldClose(WtkWindow *window, bool should_close) {
    if (window)
        window->closed = should_close;
}
