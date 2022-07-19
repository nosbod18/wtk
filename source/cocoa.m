#define GL_SILENCE_DEPRECATION
#include "../wtk.h"
#import <Cocoa/Cocoa.h>
#import <stdlib.h>

@interface ContentView : NSOpenGLView <NSWindowDelegate> {
    WtkWindow *m_window;
}
@end

struct WtkWindow {
    NSWindow *window;
    ContentView *view;
    WtkEventCallback *callback;
    bool shouldClose;
};


static int gWindowCount = 0;


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

    window->callback(window, pressed ? WtkEventType_KeyDown : WtkEventType_KeyUp, &(WtkEventData){
        .key.key = translateKeyCode([event keyCode]),
        .key.mods = translateModifiers([event modifierFlags])
    });

    } // autoreleasepool
}

static void postMouseEvent(WtkWindow *window, WtkEventType type, NSEvent const *event) {
    @autoreleasepool {

    window->callback(window, type, &(WtkEventData){
        .motion.button = [event buttonNumber],
        .motion.mods = [event modifierFlags],
        .motion.x = [event locationInWindow].x,
        .motion.y = [window->view frame].size.height - [event locationInWindow].y
    });

    } // autoreleasepool
}

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
    m_window->callback(m_window, WtkEventType_WindowClose, &(WtkEventData){0});
    return NO;
}

-(void)windowDidResize:(NSNotification *)notification {
    NSRect frame = [m_window->view frame];
    m_window->callback(m_window, WtkEventType_WindowResize, &(WtkEventData){
        .resize.width = frame.size.width,
        .resize.height = frame.size.height,
    });
}

-(void)keyDown:(NSEvent *)event             { postKeyEvent(m_window, event, true); }
-(void)keyUp:(NSEvent *)event               { postKeyEvent(m_window, event, false); }
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

-(void)scrollWheel:(NSEvent *)event {
    m_window->callback(m_window, WtkEventType_MouseScroll, &(WtkEventData){
        .scroll.dx = [event scrollingDeltaX],
        .scroll.dy = [event scrollingDeltaY]
    });
}

@end

static void createMenubar(void) {
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
}

WtkWindow *wtkCreateWindow(WtkWindowDesc const *desc) {
    @autoreleasepool {

    if (!gWindowCount) {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];
        createMenubar();
        [NSApp finishLaunching];
    }

    WtkWindowDesc actual = {
        .callback   = desc->callback ? desc->callback : defaultEventCallback,
        .title      = desc->title  ? desc->title  : "Untitled",
        .width      = desc->width  ? desc->width  : 640,
        .height     = desc->height ? desc->height : 480,
        .fullscreen = desc->fullscreen,
        .hidden     = desc->hidden
    };

    unsigned styleMask = NSWindowStyleMaskTitled  | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable;
    NSRect frame = NSMakeRect(0, 0, actual.width, actual.height);

    WtkWindow *window = malloc(sizeof *window);

    window->window      = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
    window->view        = [[ContentView alloc] initWithFrame:frame window:window];
    window->callback    = actual.callback;
    window->shouldClose = false;

    [window->window setContentView:window->view];
    [window->window setDelegate:window->view];
    [window->window setAcceptsMouseMovedEvents:YES];
    [window->window setTitle:@(actual.title)];
    [window->window setRestorable:NO];

    [window->window center];
    [window->window makeFirstResponder:window->view];
    [window->window makeKeyAndOrderFront:nil];
    [window->window orderFront:nil];

    [[window->view openGLContext] makeCurrentContext];

    gWindowCount++;
    return window;

    } // autoreleasepool
}

void wtkDeleteWindow(WtkWindow *window) {
    @autoreleasepool {

    [window->window setDelegate:nil];
    [window->view release];
    window->view = nil;

    [window->window close];
    window->window = nil;

    free(window);
    gWindowCount--;

    if (!gWindowCount)
        [NSApp terminate:nil];

    } // autoreleasepool
}

void wtkPollEvents(void) {
    @autoreleasepool {

    while (true) {
        NSEvent *event = [NSApp
            nextEventMatchingMask:NSEventMaskAny
                        untilDate:nil
                           inMode:NSDefaultRunLoopMode
                          dequeue:YES];
        if (!event)
            break;

        [NSApp sendEvent:event];
    }

    } // autoreleasepool
}

void wtkSwapBuffers(WtkWindow const *window) {
    if (window)
        [[window->view openGLContext] flushBuffer];
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

    return window ? [window->window isZoomed] : false;

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

