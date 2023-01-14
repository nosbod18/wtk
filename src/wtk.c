#include "wtk.h"
#include <stdlib.h> // calloc, free

#if defined(__APPLE__) && defined(__OBJC__)
#import <Cocoa/Cocoa.h>

@interface CocoaApp : NSObject <NSApplicationDelegate>
@end

@interface CocoaView : NSOpenGLView <NSWindowDelegate>
- (id)initWithFrame:(NSRect)frame andWindow:(WtkWindow *)window;
@end

struct WtkWindow {
    WtkWindowDesc desc;
    NSWindow *window;
    CocoaView *view;
    bool closed;
};

static struct {
    CocoaApp *app;
    int num_windows;
} _wtk = {0};

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

typedef GLXContext GlXCreateContextAttribsARBProc(Display *, GLXFBConfig, GLXContext, Bool, int const *);

struct WtkWindow {
    WtkWindowDesc desc;
    Window window;
    GLXContext context;
    bool closed;
};

static struct {
    GlXCreateContextAttribsARBProc *glx_create_ctx_attribs;
    Display *display;
    XContext context;
    Visual *visual;
    Window root;
    Colormap colormap;
    Atom wm_delwin;
    int screen;
    int depth;
    int num_windows;
} _wtk = {0};

#endif // __APPLE__ || __linux__

///////////////////////////////////////////////////////////////////////////////
/// Common

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

void WtkGetWindowRect(WtkWindow const *window, int *x, int *y, int *w, int *h) {
    if (x) *x = window ? window->desc.x : -1;
    if (y) *y = window ? window->desc.y : -1;
    if (w) *w = window ? window->desc.w : -1;
    if (h) *h = window ? window->desc.h : -1;
}

bool WtkGetWindowShouldClose(WtkWindow const *window) {
    return window ? window->closed : true;
}

///////////////////////////////////////////////////////////////////////////////
/// Cocoa

#if defined(__APPLE__) && defined(__OBJC__)

///////////////////////////////////////
// Private API

void postKeyOrButtonEvent(WtkWindow *window, WtkEventType type, NSEvent *event) {
    bool isKeyEvent = (type == WtkEventType_KeyDown || type == WtkEventType_KeyUp);
    window->desc.event_handler(window, type, &(WtkEvent){
        .key.code = isKeyEvent ? [event keyCode] : [event buttonNumber],
        .key.sym  = isKeyEvent ? [event keyCode] : [event buttonNumber],
        .key.mods = [event modifierFlags],
        .key.x    = [event locationInWindow].x,
        .key.y    = [event locationInWindow].y,
    });
}

void postMotionOrScrollEvent(WtkWindow *window, WtkEventType type, NSEvent *event) {
    window->desc.event_handler(window, type, &(WtkEvent){
        .motion.dx = [event deltaX],
        .motion.dy = [event deltaY],
    });
}

void postOtherEvent(WtkWindow *window, WtkEventType type) {
    window->desc.event_handler(window, type, &(WtkEvent){0});
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
    m_window->closed = true;
    postOtherEvent(m_window, WtkEventType_WindowClose);
    return NO;
}

-(void)keyDown:(NSEvent *)event         { postKeyOrButtonEvent(m_window, WtkEventType_KeyDown,   event); }
-(void)keyUp:(NSEvent *)event           { postKeyOrButtonEvent(m_window, WtkEventType_KeyUp,     event); }
-(void)mouseDown:(NSEvent *)event       { postKeyOrButtonEvent(m_window, WtkEventType_MouseDown, event); }
-(void)mouseUp:(NSEvent *)event         { postKeyOrButtonEvent(m_window, WtkEventType_MouseUp, event);   }
-(void)rightMouseDown:(NSEvent *)event  { [self mouseDown:event]; }
-(void)rightMouseUp:(NSEvent *)event    { [self mouseUp:event];   }
-(void)otherMouseDown:(NSEvent *)event  { [self mouseDown:event]; }
-(void)otherMouseUp:(NSEvent *)event    { [self mouseUp:event];   }
-(void)mouseEntered:(NSEvent *)event    { postOtherEvent(m_window, WtkEventType_MouseEnter); }
-(void)mouseExited:(NSEvent *)event     { postOtherEvent(m_window, WtkEventType_MouseLeave); }
-(void)mouseMoved:(NSEvent *)event      { postMotionOrScrollEvent(m_window, WtkEventType_MouseMotion, event); }
-(void)scrollWheel:(NSEvent *)event     { postMotionOrScrollEvent(m_window, WtkEventType_MouseScroll, event); }


@end

static float translateYCoordinate(float y) {
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}

static bool init(void) {
    @autoreleasepool {

    [NSApplication sharedApplication];
    _wtk.app = [[CocoaApp alloc] init];

    [NSApp setDelegate:_wtk.app];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp finishLaunching];

    return true;

    } // autoreleasepool
}

static void terminate(void) {
    @autoreleasepool {

    [NSApp terminate:nil];
    [_wtk.app release];

    } // autoreleasepool
}

///////////////////////////////////////
// Public API

WtkWindow *WtkCreateWindow(WtkWindowDesc const *desc) {
    @autoreleasepool {

    if (!desc || (_wtk.num_windows == 0 && !init()))
        return NULL;

    WtkWindow *window = calloc(1, sizeof *window);
    if (!window) return NULL;

    window->desc = *desc;
    validateDesc(window);

    unsigned styleMask = NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable;
    NSRect frame = NSMakeRect(0, 0, window->desc.w, window->desc.h);

    window->window = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
    if (!window->window) return NULL;

    window->view = [[CocoaView alloc] initWithFrame:frame andWindow:window];
    if (!window->view) return NULL;

    [window->window setContentView:window->view];
    [window->window setDelegate:window->view];
    [window->window makeFirstResponder:window->view];

    [window->window setAcceptsMouseMovedEvents:YES];
    [window->window setRestorable:NO];
    [window->window center];
    [window->window makeKeyAndOrderFront:nil];
    [window->window orderFront:nil];

    return window;

    } // autoreleasepool
}

void WtkMakeCurrent(WtkWindow *window) {
    @autoreleasepool {

    [[window->view openGLContext] makeCurrentContext];

    } // autoreleasepool
}

void WtkSwapBuffers(WtkWindow *window) {
    @autoreleasepool {

    [[window->view openGLContext] flushBuffer];

    } // autoreleasepool
}

void WtkPollEvents(void) {
    @autoreleasepool {

    while (true) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (!event) break;

        [NSApp sendEvent:event];
    }

    } // autoreleasepool
}

void WtkDeleteWindow(WtkWindow *window) {
    @autoreleasepool {

    if (!window)
        return;

    [window->view release];
    [window->window setDelegate:nil];
    [window->window close];
    free(window);

    if (--_wtk.num_windows == 0)
        terminate();

    } // autoreleasepool
}

void WtkSetWindowOrigin(WtkWindow *window, int x, int y) {
    @autoreleasepool {

    if (!window)
        return;

    NSRect rect  = NSMakeRect(x, translateYCoordinate(y + [window->view frame].size.height - 1), 0, 0);
    NSRect frame = [window->window frameRectForContentRect:rect];
    [window->window setFrameOrigin:frame.origin];
    window->desc.x = x;
    window->desc.y = y;

    } // autoreleasepool
}

void WtkSetWindowSize(WtkWindow *window, int w, int h) {
    @autoreleasepool {

    if (!window)
        return;

    NSRect contentRect = [window->window contentRectForFrameRect:[window->window frame]];
    contentRect.origin.y += contentRect.size.height - h;
    contentRect.size = NSMakeSize(w, h);
    [window->window setFrame:[window->window frameRectForContentRect:contentRect] display:YES];
    window->desc.w = w;
    window->desc.h = h;

    } // autoreleasepool
}

void WtkSetWindowTitle(WtkWindow *window, char const *title) {
    @autoreleasepool {

    if (!window)
        return;

    [window->window setTitle:@(title)];
    window->desc.title = title;

    } // autoreleasepool
}

void WtkSetWindowShouldClose(WtkWindow *window, bool should_close) {
    if (window)
        window->closed = should_close;
}

///////////////////////////////////////////////////////////////////////////////
/// X11

#elif defined(__linux__)

///////////////////////////////////////
// Private API

static bool init(void) {
    if (!(_wtk.display = XOpenDisplay(NULL)))
        return false;

    _wtk.screen = DefaultScreen(_wtk.display);
    _wtk.root   = RootWindow(_wtk.display, _wtk.screen);
    _wtk.visual = DefaultVisual(_wtk.display, _wtk.screen);

    if (!(_wtk.colormap = XCreateColormap(_wtk.display, _wtk.root, _wtk.visual, AllocNone)))
        return false;

    _wtk.depth = DefaultDepth(_wtk.display, _wtk.screen);

    if (!(_wtk.wm_delwin = XInternAtom(_wtk.display, "WM_DELETE_WINDOW", 0)))
        return false;

    _wtk.glx_create_ctx_attribs = (GlXCreateContextAttribsARBProc *)glXGetProcAddressARB((GLubyte const *)"glx_create_ctx_attribs");

    return true;
}

static void terminate(void) {
    XFreeColormap(_wtk.display, _wtk.colormap);
    XCloseDisplay(_wtk.display);
}

///////////////////////////////////////
// Public API

WtkWindow *WtkCreateWindow(WtkWindowDesc const *desc) {
    if (!desc || (_wtk.num_windows == 0 && !init()))
        return NULL;

    WtkWindow *window = calloc(1, sizeof *window);
    if (!window) return NULL;

    window->desc = *desc;
    validateDesc(window);

    XSetWindowAttributes swa = {
        .event_mask = StructureNotifyMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|KeyPressMask|KeyReleaseMask|EnterWindowMask|LeaveWindowMask|FocusChangeMask|ExposureMask,
        .colormap = _wtk.colormap
    };

    window->window = XCreateWindow(
        _wtk.display, _wtk.root,
        window->desc.x, window->desc.y, window->desc.w, window->desc.h,
        0, _wtk.depth, InputOutput,
        _wtk.visual, CWColormap | CWEventMask, &swa
    );
    if (!native->window) return NULL;

    if (!XSetWMProtocols(_wtk.display, native->window, &_wtk.wm_delwin, 1))
        return NULL;

    GLint vis_attribs[] = {
        GLX_RENDER_TYPE,  GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, 1,
        None
    };

    int fbcount = 0;
    GLXFBConfig *fbc = glXChooseFBConfig(_wtk.display, _wtk.screen, vis_attribs, &fbcount);
    if (!fbc || !fbcount) return NULL;

    GLint ctx_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };

    if (_wtk.glx_create_ctx_attribs)
        native->context = _wtk.glx_create_ctx_attribs(_wtk.display, fbc[0], NULL, 1, ctx_attribs);
    else
        native->context = glXCreateNewContext(_wtk.display, fbc[0], GLX_RGBA_TYPE, NULL, 1);

    XMapWindow(_wtk.display, native->window);
    XFlush(_wtk.display);

    _wtk.num_windows++;
    return window;
}

void WtkMakeCurrent(WtkWindow *window) {
    if (!window) return;
    glXMakeContextCurrent(_wtk.display, window->window, window->window, window->context);
}

void WtkSwapBuffers(WtkWindow window) {
    if (!window) return;
    glXSwapBuffers(_wtk.display, window->window);
}

void WtkPollEvents(void) {
    XEvent event;
    while (XPending(_wtk.display)) {
        XNextEvent(_wtk.display, &event);
        switch (event.type) {
            case ConfigureNotify: {
                window->desc.x = event.xconfigure.x;
                window->desc.y = event.xconfigure.y;
                window->desc.w = event.xconfigure.width;
                window->desc.h = event.xconfigure.height;
                window->event_handler(window, WtkEventType_Resize, &(WtkEvent){0});
            } break;

            case KeyPress:
            case KeyRelease: {
                window->event_handler(window, event.type == KeyPress ? WtkEventType_KeyDown : WtkEventType_KeyUp, &(WtkEvent){
                    .key.code = event.xkey.keycode, // TODO: Translate
                    .key.sym  = event.xkey.keycode,
                    .key.mods = event.xkey.state,
                    .key.x    = event.xkey.x,
                    .key.y    = event.xkey.y
                });
            } break;

            case ButtonPress:
            case ButtonRelease: {
                window->event_handler(window, event.type == ButtonPress ? WtkEventType_MouseDown : WtkEventType_MouseUP, &(WtkEvent){
                    .button.code = event.xbutton.button, // TODO: Translate
                    .button.sym  = event.xbutton.button,
                    .button.mods = event.xbutton.state,
                    .button.x    = event.xbutton.x,
                    .button.y    = event.xbutton.y
                });
            } break;

            case MotionNotify: {
                window->event_handler(window, WtkEventType_MouseMotion, &(WtkEvent){
                    .motion.dx = event.xmotion.x,
                    .motion.dy = event.xmotion.y,
                });
            } break;

            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == _wtk.wm_delwin) {
                    WtkSetWindowShouldClose(window, true); // Must come before window->event_handler()
                    window->event_handler(window, WtkEventType_WindowClose, &(WtkEvent){0});
                }
            } break;

            case MapNotify:   { window->event_handler(window, WtkEventType_WindowFocusIn, &(WtkEvent){0});    } break;
            case UnmapNotify: { window->event_handler(window, WtkEventType_WindowFocusOut, &(WtkEvent){0});   } break;
            case EnterNotify: { window->event_handler(window, WtkEventType_WindowMouseEnter, &(WtkEvent){0}); } break;
            case LeaveNotify: { window->event_handler(window, WtkEventType_WindowMouseLeave, &(WtkEvent){0}); } break;

            default: {
            } break;
        }
    }
}

void WtkDeleteWindow(WtkWindow *window) {
    if (!window)
        return;

    glXDestroyContext(_wtk.display, window->context);
    XDestroyWindow(_wtk.display, window->window);
    free(native);

    if (--_wtk.num_windows == 0)
        terminate();
}

void WtkSetWindowOrigin(WtkWindow *window, int x, int y) {
    if (!window) return;
    XMoveWindow(_wtk.display, window->window, x, y);
    window->desc.x = x;
    window->desc.y = y;
}

void WtkSetWindowSize(WtkWindow *window, int w, int h) {
    if (!window) return;
    XResizeWindow(_wtk.display, window->window, (unsigned int)w, (unsigned int)h);
    window->desc.w = w;
    window->desc.h = h;
}

void WtkSetWindowTitle(WtkWindow *window, char const *title) {
    if (!window) return;
    XStoreName(_wtk.display, window->window, title);
    window->desc.title = title;
}

void WtkSetWindowShouldClose(WtkWindow *window, bool should_close) {
    if (window)
        window->closed = should_close;
}

#endif // __APPLE__ || __linux__