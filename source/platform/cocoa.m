#ifdef WTK_USE_COCOA

#define GL_SILENCE_DEPRECATION
#import "wtk/wtk.h"
#import "platform.h"
#import "plugins/log/log.h"
#import <Cocoa/Cocoa.h>
#import <stdbool.h>

///////////////////////////////////////////////////////////////////////////////
// Helper functions ///////////////////////////////////////////////////////////

static float translateYCoordinate(float y) {
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}

static int translateKeyCode(int keyCode) {
    return keyCode; // TODO
}

static int translateModifiers(int mods) {
    return mods; // TODO
}

static void postKeyEvent(WtkWindow *window, WtkEventType type, NSEvent const *event) {
    @autoreleasepool {

    window->onEvent(&(WtkEvent){
        .window = window,
        .type = type,
        .time = [event timestamp],
        .location = {
            .x = [event locationInWindow].x,
            .y = [window->cocoa.view frame].size.height - [event locationInWindow].y,
        },
        .key = translateKeyCode([event keyCode]),
        .sym = [event keyCode],
        .mods = translateModifiers([event modifierFlags]),
        .isRepeat = [event isARepeat],
    });

    } // autoreleasepool
}

static void postButtonEvent(WtkWindow *window, WtkEventType type, NSEvent const *event) {
    window->onEvent(&(WtkEvent){
        .window = window,
        .type = type,
        .time = [event timestamp],
        .location = {
            .x = [event locationInWindow].x,
            .y = [window->cocoa.view frame].size.height - [event locationInWindow].y,
        },
        .button = [event buttonNumber],
        .sym = [event buttonNumber],
        .mods = translateModifiers([event modifierFlags]),
        .isRepeat = [event isARepeat],
        .clicks = [event clickCount],
    });
}

static void postMotionOrScrollEvent(WtkWindow *window, WtkEventType type, NSEvent const *event) {
    window->onEvent(&(WtkEvent){
        .window = window,
        .type = type,
        .time = [event timestamp],
        .location = {
            .x = [event locationInWindow].x,
            .y = [window->cocoa.view frame].size.height - [event locationInWindow].y,
        },
        .mods = translateModifiers([event modifierFlags]),
        .delta = {
            .x = [event deltaX],
            .y = [event deltaY],
            .z = [event deltaZ],
        }
    });
}

static void postOtherEvent(WtkWindow *window, WtkEventType type) {
    window->onEvent(&(WtkEvent){
        .window = window,
        .type = type
    });
}

///////////////////////////////////////////////////////////////////////////////
// Class functions ////////////////////////////////////////////////////////////

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

-(void)windowDidBecomeKey:(NSNotification *)notification {
    postOtherEvent(m_window, WtkEventType_WindowFocusIn);
}

-(void)windowDidResignKey:(NSNotification *)notification {
    postOtherEvent(m_window, WtkEventType_WindowFocusOut);
}

-(void)windowDidResize:(NSNotification *)notification {
    postOtherEvent(m_window, WtkEventType_WindowResize);
}

-(BOOL)windowShouldClose:(NSNotification *)notification {
    // wtkSetWindowShouldClose must come first since the user may decide to cancel the close request when they handle the event
    wtkSetWindowShouldClose(m_window, true);
    postOtherEvent(m_window, WtkEventType_WindowClose);
    return NO;
}

-(void)mouseDown:(NSEvent *)event {
    postButtonEvent(m_window, WtkEventType_MouseDown, event);
}

-(void)mouseUp:(NSEvent *)event {
    postButtonEvent(m_window, WtkEventType_MouseUp, event);
}

-(void)mouseDragged:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WtkEventType_MouseMotion, event);
}

-(void)scrollWheel:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WtkEventType_MouseScroll, event);
}

-(void)rightMouseDown:(NSEvent *)event {
    postButtonEvent(m_window, WtkEventType_MouseDown, event);
}

-(void)rightMouseUp:(NSEvent *)event {
    postButtonEvent(m_window, WtkEventType_MouseUp, event);
}

-(void)rightMouseDragged:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WtkEventType_MouseMotion, event);
}

-(void)otherMouseDown:(NSEvent *)event {
    postButtonEvent(m_window, WtkEventType_MouseDown, event);
}

-(void)otherMouseUp:(NSEvent *)event {
    postButtonEvent(m_window, WtkEventType_MouseUp, event);
}

-(void)otherMouseDragged:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WtkEventType_MouseMotion, event);
}

-(void)mouseMoved:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WtkEventType_MouseMotion, event);
}

-(void)keyDown:(NSEvent *)event {
    postKeyEvent(m_window, WtkEventType_KeyDown, event);
}

-(void)keyUp:(NSEvent *)event {
    postKeyEvent(m_window, WtkEventType_KeyUp, event);
}

@end

///////////////////////////////////////////////////////////////////////////////
// Platform functions /////////////////////////////////////////////////////////

bool platformStart(void) {
    @autoreleasepool {

    [NSApplication sharedApplication];

    WTK.cocoa.appDelegate = [[AppDelegate alloc] init];
    if (!WTK.cocoa.appDelegate) {
        error("Failed to initialize AppDelegate");
        return false;
    }

    [NSApp setDelegate:WTK.cocoa.appDelegate];
    [NSApp finishLaunching];

    WTK.cocoa.bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
    if (!WTK.cocoa.bundle) {
        error("Failed to locate OpenGL framework");
        return false;
    }

    return true;

    } // autoreleasepool
}

void platformStop(void) {
    @autoreleasepool {

    [NSApp terminate:nil];
    [WTK.cocoa.appDelegate release];

    } // autoreleasepool
}

bool platformCreateWindow(WtkWindow *window) {
    @autoreleasepool {

    window->cocoa.window = [[NSWindow alloc]
                                initWithContentRect:NSMakeRect(0, 0, window->w, window->h)
                                          styleMask:NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable
                                            backing:NSBackingStoreBuffered
                                              defer:NO];
    if (!window->cocoa.window) {
        error("Failed to create NSWindow");
        return false;
    }

    [window->cocoa.window setAcceptsMouseMovedEvents:YES];
    [window->cocoa.window setRestorable:NO];
    [window->cocoa.window center];
    [window->cocoa.window makeKeyAndOrderFront:nil];
    [window->cocoa.window orderFront:nil];

    return true;

    } // autoreleasepool
}

void platformDeleteWindow(WtkWindow *window) {
    @autoreleasepool {

    if (!window->cocoa.window)
        return;

    [window->cocoa.window close];
    window->cocoa.window = nil;

    } // autoreleasepool
}

bool platformCreateContext(WtkWindow *window, WtkWindowDesc const *desc) {
    @autoreleasepool {
    (void)desc; // TODO

    window->cocoa.view = [[ContentView alloc] initWithFrame:NSMakeRect(0, 0, window->w, window->h) window:window];
    if (!window->cocoa.view) {
        error("Failed to create ContentView");
        return false;
    }

    [window->cocoa.window setContentView:window->cocoa.view];
    [window->cocoa.window setDelegate:window->cocoa.view];
    [window->cocoa.window makeFirstResponder:window->cocoa.view];

    return true;

    } // autoreleasepool
}

void platformDeleteContext(WtkWindow *window) {
    @autoreleasepool {

    if (!window->cocoa.view)
        return;

    if (window->cocoa.window)
        [window->cocoa.window setDelegate:nil];

    [window->cocoa.view release];
    window->cocoa.view = nil;

    } // autoreleasepool
}

void platformMakeCurrent(WtkWindow const *window) {
    @autoreleasepool {

    if (window)
        [[window->cocoa.view openGLContext] makeCurrentContext];
    else
        [NSOpenGLContext clearCurrentContext];

    } // autoreleasepool
}

void platformSwapBuffers(WtkWindow const *window) {
    @autoreleasepool {

    [[window->cocoa.view openGLContext] flushBuffer];

    } // autoreleasepool
}

void platformPollEvents(void) {
    @autoreleasepool {

    while (true) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (!event)
            break;

        [NSApp sendEvent:event];
    }

    } // autoreleasepool
}

void platformSetWindowPos(WtkWindow *window, int x, int y) {
    @autoreleasepool {

    NSRect rect  = NSMakeRect(x, translateYCoordinate(y + [window->cocoa.view frame].size.height - 1), 0, 0);
    NSRect frame = [window->cocoa.window frameRectForContentRect:rect];
    [window->cocoa.window setFrameOrigin:frame.origin];

    } // autoreleasepool
}

void platformSetWindowSize(WtkWindow *window, int w, int h) {
    @autoreleasepool {

    NSRect contentRect = [window->cocoa.window contentRectForFrameRect:[window->cocoa.window frame]];
    contentRect.origin.y += contentRect.size.height - h;
    contentRect.size = NSMakeSize(w, h);
    [window->cocoa.window setFrame:[window->cocoa.window frameRectForContentRect:contentRect] display:YES];

    } // autoreleasepool
}

void platformSetWindowTitle(WtkWindow *window, char const *title) {
    @autoreleasepool {

    [window->cocoa.window setTitle:@(title)];

    } // autoreleasepool
}

WtkGLProc *platformGetProcAddress(char const *name) {
    CFStringRef symbolName = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingASCII);
    WtkGLProc *symbol = CFBundleGetFunctionPointerForName(WTK.cocoa.bundle, symbolName);
    CFRelease(symbolName);
    return symbol;
}

#endif // WTK_USE_COCOA