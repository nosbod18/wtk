#import "wtk/wtk.h"
#import "platform.h"

@interface CocoaApp : NSObject <NSApplicationDelegate>
@end

static CocoaApp *g_app = nil;

static void postKeyOrButtonEvent(WtkWindow *window, WtkEventType type, NSEvent *event) {
    bool isKeyEvent = (type == WtkEventType_KeyDown || type == WtkEventType_KeyUp);
    window->desc.event_handler(window, type, &(WtkEvent){
        .key.code = isKeyEvent ? [event keyCode] : [event buttonNumber],
        .key.sym  = isKeyEvent ? [event keyCode] : [event buttonNumber],
        .key.mods = [event modifierFlags],
        .key.x    = [event locationInWindow].x,
        .key.y    = [event locationInWindow].y,
    });
}

static void postMotionOrScrollEvent(WtkWindow *window, WtkEventType type, NSEvent *event) {
    window->desc.event_handler(window, type, &(WtkEvent){
        .motion.dx = [event deltaX],
        .motion.dy = [event deltaY],
    });
}

static void postOtherEvent(WtkWindow *window, WtkEventType type) {
    window->desc.event_handler(window, type, &(WtkEvent){0});
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

bool platformInit(void) {
    @autoreleasepool {

    [NSApplication sharedApplication];
    g_app = [[CocoaApp alloc] init];

    [NSApp setDelegate:g_app];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    [NSApp finishLaunching];

    return true;

    }
}

void platformTerminate(void) {
    @autoreleasepool {

    [NSApp terminate:nil];
    [g_app release];

    }
}

bool platformCreateWindow(WtkWindow *window) {
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

void platformMakeCurrent(WtkWindow *window) {
    @autoreleasepool {

    [[window->view openGLContext] makeCurrentContext];

    }
}

void platformSwapBuffers(WtkWindow *window) {
    @autoreleasepool {

    [[window->view openGLContext] flushBuffer];

    }
}

void platformPollEvents(void) {
    @autoreleasepool {

    while (true) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (!event) break;

        [NSApp sendEvent:event];
    }

    }
}

void platformDeleteWindow(WtkWindow *window) {
    @autoreleasepool {

    if (window->window) {
        if (window->view)
            [window->view release];
        [window->window setDelegate:nil];
        [window->window close];
    }

    }
}

void platformSetWindowOrigin(WtkWindow *window, int x, int y) {
    @autoreleasepool {

    NSRect rect  = NSMakeRect(x, translateYCoordinate(y + [window->view frame].size.height - 1), 0, 0);
    NSRect frame = [window->window frameRectForContentRect:rect];
    [window->window setFrameOrigin:frame.origin];

    }
}

void platformSetWindowSize(WtkWindow *window, int w, int h) {
    @autoreleasepool {

    NSRect contentRect = [window->window contentRectForFrameRect:[window->window frame]];
    contentRect.origin.y += contentRect.size.height - h;
    contentRect.size = NSMakeSize(w, h);
    [window->window setFrame:[window->window frameRectForContentRect:contentRect] display:YES];

    }
}

void platformSetWindowTitle(WtkWindow *window, char const *title) {
    @autoreleasepool {

    [window->window setTitle:@(title)];

    }
}
