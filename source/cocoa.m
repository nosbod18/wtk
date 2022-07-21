#define GL_SILENCE_DEPRECATION
#include "../wtk.h"
#import <Cocoa/Cocoa.h>
#import <stdlib.h>
#import <stdio.h>  // printf

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@interface ContentView : NSOpenGLView <NSWindowDelegate> {
    WtkWindow *m_window;
}
@end

struct WtkWindow {
    NSWindow *window;
    ContentView *view;
    WtkEventCallback *onEvent;
    bool shouldClose;
};

static struct {
    AppDelegate *appDelegate;
    WtkErrorCallback *onError;
    void *(*alloc)(size_t size);
    void (*free)(void *ptr);
} WTK;

static float translateYCoordinate(float y) {
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}

static void defaultEventCallback(WtkWindow *window, WtkEventType type, WtkEventData const *data) {
    (void)window; (void)type; (void)data;
}

static int translateKeyCode(int keyCode) {
    return keyCode; // TODO
}

static int translateModifiers(int mods) {
    return mods; // TODO
}

static void postKeyEvent(WtkWindow *window, NSEvent const *event, bool pressed) {
    @autoreleasepool {

    window->onEvent(window, pressed ? WtkEventType_KeyDown : WtkEventType_KeyUp, &(WtkEventData){
        .key.keycode = translateKeyCode([event keyCode]),
        .key.scancode = [event keyCode],
        .key.mods = translateModifiers([event modifierFlags])
    });

    } // autoreleasepool
}

static void postMouseEvent(WtkWindow *window, WtkEventType type, NSEvent const *event) {
    @autoreleasepool {

    window->onEvent(window, type, &(WtkEventData){
        .motion.button = [event buttonNumber],
        .motion.mods = [event modifierFlags],
        .motion.x = [event locationInWindow].x,
        .motion.y = [window->view frame].size.height - [event locationInWindow].y
    });

    } // autoreleasepool
}

static bool validateWtkDesc(WtkDesc *desc) {
    if (!desc)
        return false;

    if (!desc->onError) desc->onError = printf;
    if (!desc->alloc)   desc->alloc   = malloc;
    if (!desc->free)    desc->free    = free;

    return true;
}

static bool validateWtkWindowDesc(WtkWindowDesc *desc) {
    if (!desc)
        return false;

    if (!desc->onEvent) desc->onEvent   = defaultEventCallback;
    if (!desc->title)   desc->title     = "Untitled";
    if (!desc->width)   desc->width     = 640;
    if (!desc->height)  desc->height    = 480;
    if (!desc->flags)   desc->flags     = WtkWindowFlags_Closable | WtkWindowFlags_Resizable | WtkWindowFlags_Titled | WtkWindowFlags_Centered;

    return true;
}

@implementation AppDelegate
-(void)applicationWillFinishLaunching:(NSNotification *)notification {
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];

    id appMenu = [[NSMenu new] autorelease];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];

    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}
@end

@implementation ContentView
- (id)initWithFrame:(NSRect)frame window:(WtkWindow *)window {
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

    if ((self = [super initWithFrame:frame pixelFormat:[[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease]]))
        m_window = window;

    return self;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

-(BOOL)windowShouldClose:(NSNotification *)notification {
    wtkSetWindowShouldClose(m_window, true);
    m_window->onEvent(m_window, WtkEventType_WindowClose, &(WtkEventData){0});
    printf("hi\n");
    return NO;
}

-(void)windowDidResize:(NSNotification *)notification {
    NSRect frame = [m_window->view frame];
    m_window->onEvent(m_window, WtkEventType_WindowResize, &(WtkEventData){
        .resize.width = frame.size.width,
        .resize.height = frame.size.height,
    });
}

-(void)scrollWheel:(NSEvent *)event {
    m_window->onEvent(m_window, WtkEventType_MouseScroll, &(WtkEventData){
        .scroll.dx = [event scrollingDeltaX],
        .scroll.dy = [event scrollingDeltaY]
    });
}

-(void)mouseDown:(NSEvent *)event           { postMouseEvent(m_window, WtkEventType_MouseDown, event); }
-(void)mouseUp:(NSEvent *)event             { postMouseEvent(m_window, WtkEventType_MouseUp, event); }
-(void)mouseDragged:(NSEvent *)event        { postMouseEvent(m_window, WtkEventType_MouseMotion, event); }
-(void)rightMouseDown:(NSEvent *)event      { postMouseEvent(m_window, WtkEventType_MouseDown, event); }
-(void)rightMouseUp:(NSEvent *)event        { postMouseEvent(m_window, WtkEventType_MouseUp, event); }
-(void)rightMouseDragged:(NSEvent *)event   { postMouseEvent(m_window, WtkEventType_MouseMotion, event); }
-(void)otherMouseDown:(NSEvent *)event      { postMouseEvent(m_window, WtkEventType_MouseDown, event); }
-(void)otherMouseUp:(NSEvent *)event        { postMouseEvent(m_window, WtkEventType_MouseUp, event); }
-(void)otherMouseDragged:(NSEvent *)event   { postMouseEvent(m_window, WtkEventType_MouseMotion, event); }
-(void)mouseMoved:(NSEvent *)event          { postMouseEvent(m_window, WtkEventType_MouseMotion, event); }
-(void)keyDown:(NSEvent *)event             { postKeyEvent(m_window, event, true); }
-(void)keyUp:(NSEvent *)event               { postKeyEvent(m_window, event, false); }
@end

bool wtkInit(WtkDesc *desc) {
    if (!validateWtkDesc(desc))
        return false;

    WTK.onError = desc->onError;
    WTK.alloc = desc->alloc;
    WTK.free = desc->free;

    [NSApplication sharedApplication];
    WTK.appDelegate = [[AppDelegate alloc] init];
    [NSApp setDelegate:WTK.appDelegate];
    [NSApp finishLaunching];

    return true;
}

void wtkQuit(void) {
    [NSApp terminate:nil];
    [WTK.appDelegate release];
}

void wtkMakeCurrent(WtkWindow const *window) {
    if (window && window->view)
        [[window->view openGLContext] makeCurrentContext];
    else
        [NSOpenGLContext clearCurrentContext];
}

void wtkSwapBuffers(WtkWindow const *window) {
    if (window && window->view)
        [[window->view openGLContext] flushBuffer];
}

void wtkPollEvents(void) {
    @autoreleasepool {

    while (true) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (!event)
            break;

        [NSApp sendEvent:event];
    }

    } // autoreleasepool
}

WtkWindow *wtkCreateWindow(WtkWindowDesc *desc) {
    @autoreleasepool {

    if (!validateWtkWindowDesc(desc))
        return false;

    unsigned styleMask = NSWindowStyleMaskMiniaturizable;
    if (desc->flags & WtkWindowFlags_Titled)     styleMask |= NSWindowStyleMaskTitled;
    if (desc->flags & WtkWindowFlags_Closable)   styleMask |= NSWindowStyleMaskClosable;
    if (desc->flags & WtkWindowFlags_Resizable)  styleMask |= NSWindowStyleMaskResizable;

    NSRect frame = NSMakeRect(0, 0, desc->width, desc->height);

    WtkWindow *window = WTK.alloc(sizeof *window);
    if (!window) {
        WTK.onError("Failed to allocated window\n");
        return NULL;
    }

    window->window = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
    if (!window->window) {
        WTK.onError("Failed to create NSWindow\n");
        wtkDeleteWindow(window);
        return NULL;
    }

    window->view = [[ContentView alloc] initWithFrame:frame window:window];
    if (!window->view) {
        WTK.onError("Failed to create ContentView\n");
        wtkDeleteWindow(window);
        return NULL;
    }

    window->onEvent = desc->onEvent;
    window->shouldClose = false;

    [window->window setContentView:window->view];
    [window->window setDelegate:window->view];
    [window->window setAcceptsMouseMovedEvents:YES];
    [window->window setTitle:@(desc->title)];
    [window->window setRestorable:NO];

    [window->window center];
    [window->window makeFirstResponder:window->view];
    [window->window makeKeyAndOrderFront:nil];
    [window->window orderFront:nil];

    return window;

    } // autoreleasepool
}

void wtkDeleteWindow(WtkWindow *window) {
    @autoreleasepool {

    if (!window)
        return;

    if (window->window) {
        if (window->view) {
            [window->window setDelegate:nil];
            [window->view release];
            window->view = nil;
        }

        [window->window close];
        window->window = nil;
    }

    WTK.free(window);

    } // autoreleasepool
}

void wtkGetWindowPos(WtkWindow const *window, int *x, int *y) {
    @autoreleasepool {

    if (x) *x = 0;
    if (y) *y = 0;

    if (window) {
        NSRect frame = [window->view frame];
        if (x) *x = frame.origin.x;
        if (y) *y = frame.origin.y;
    }

    } // autoreleasepool
}

void wtkGetWindowSize(WtkWindow const *window, int *width, int *height) {
    @autoreleasepool {

    if (width)  *width = 0;
    if (height) *height = 0;

    if (window) {
        NSRect frame = [window->view frame];
        if (width)  *width = frame.size.width;
        if (height) *height = frame.size.height;
    }

    } // autoreleasepool
}

bool wtkGetWindowVisible(WtkWindow const *window) {
    @autoreleasepool {

    (void)window;
    return true; // TODO

    } // autorelease
}

bool wtkGetWindowFullscreen(WtkWindow const *window) {
    @autoreleasepool {

    return window ? [window->window styleMask] & NSWindowStyleMaskFullScreen  : false;

    } // autoreleasepool
}

bool wtkGetWindowShouldClose(WtkWindow const *window) {
    return window ? window->shouldClose : true;
}

void wtkSetWindowPos(WtkWindow *window, int x, int y) {
    @autoreleasepool {

    NSRect rect  = NSMakeRect(x, translateYCoordinate(y + [window->view frame].size.height - 1), 0, 0);
    NSRect frame = [window->window frameRectForContentRect:rect];
    [window->window setFrameOrigin:frame.origin];

    } // autoreleasepool
}

void wtkSetWindowSize(WtkWindow *window, int width, int height) {
    @autoreleasepool {

    if (!window)
        return;

    NSRect contentRect = [window->window contentRectForFrameRect:[window->window frame]];
    contentRect.origin.y += contentRect.size.height - height;
    contentRect.size = NSMakeSize(width, height);
    [window->window setFrame:[window->window frameRectForContentRect:contentRect] display:YES];

    } // autoreleasepool
}

void wtkSetWindowTitle(WtkWindow *window, char const *title) {
    @autoreleasepool {

    if (window && title)
        [window->window setTitle:@(title)];

    } // autoreleasepool
}

void wtkSetWindowFullscreen(WtkWindow *window, bool fullscreen) {
    @autoreleasepool {

    (void)window; (void)fullscreen;
    // TODO

    } // autoreleasepool
}

void wtkSetWindowVisibile(WtkWindow *window, bool visible) {
    @autoreleasepool {

    (void)window; (void)visible;
    // TODO

    } // autoreleasepool
}

void wtkSetWindowShouldClose(WtkWindow *window, bool shouldClose) {
    @autoreleasepool {

    if (window)
        window->shouldClose = shouldClose;

    } // autoreleasepool
}

