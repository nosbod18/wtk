#define GL_SILENCE_DEPRECATION
#import "../window.h"
#import "../extern/log/log.h"
#import <Cocoa/Cocoa.h>
#import <stdbool.h>

// Forward declaration
typedef struct window_cocoa_t window_cocoa_t;

@interface delegate_t : NSObject <NSApplicationDelegate>
@end

@interface view_t : NSOpenGLView <NSWindowDelegate>
- (id)initWithFrame:(NSRect)frame window:(window_cocoa_t *)window;
@end

struct window_cocoa_t {
    NSWindow *window;
    view_t *view;
};

static struct {
    delegate_t *delegate;
    CFBundleRef bundle;
    int nwindows;
} _wnd = {0};

///////////////////////////////////////////////////////////////////////////////
// Helper functions ///////////////////////////////////////////////////////////

static float translateYCoordinate(float y) {
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}

static void postKeyEvent(WndWindow *window, WndEventType type, NSEvent const *event) {
    @autoreleasepool {

    window->onEvent(&(WndEvent){
        .window = window,
        .type = type,
        .time = [event timestamp],
        .location = {
            .x = [event locationInWindow].x,
            .y = [native->view frame].size.height - [event locationInWindow].y,
        },
        .key = translateKeyCode([event keyCode]),
        .sym = [event keyCode],
        .mods = translateModifiers([event modifierFlags]),
        .isRepeat = [event isARepeat],
    });

    } // autoreleasepool
}

static void postButtonEvent(WndWindow *window, WndEventType type, NSEvent const *event) {
    window->onEvent(&(WndEvent){
        .window = window,
        .type = type,
        .time = [event timestamp],
        .location = {
            .x = [event locationInWindow].x,
            .y = [native->view frame].size.height - [event locationInWindow].y,
        },
        .button = [event buttonNumber],
        .sym = [event buttonNumber],
        .mods = translateModifiers([event modifierFlags]),
        .isRepeat = [event isARepeat],
        .clicks = [event clickCount],
    });
}

static void postMotionOrScrollEvent(WndWindow *window, WndEventType type, NSEvent const *event) {
    window->onEvent(&(WndEvent){
        .window = window,
        .type = type,
        .time = [event timestamp],
        .location = {
            .x = [event locationInWindow].x,
            .y = [native->view frame].size.height - [event locationInWindow].y,
        },
        .mods = translateModifiers([event modifierFlags]),
        .delta = {
            .x = [event deltaX],
            .y = [event deltaY],
            .z = [event deltaZ],
        }
    });
}

static void postOtherEvent(WndWindow *window, WndEventType type) {
    window->onEvent(&(WndEvent){
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

@implementation view_t {
    window_cocoa_t *m_window;
}
- (id)initWithFrame:(NSRect)frame window:(window_cocoa_t *)window {
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
    postOtherEvent(m_window, WndEventType_WindowFocusIn);
}

-(void)windowDidResignKey:(NSNotification *)notification {
    postOtherEvent(m_window, WndEventType_WindowFocusOut);
}

-(void)windowDidResize:(NSNotification *)notification {
    postOtherEvent(m_window, WndEventType_WindowResize);
}

-(BOOL)windowShouldClose:(NSNotification *)notification {
    // WndSetWindowShouldClose must come first since the user may decide to cancel the close request when they handle the event
    WndSetWindowShouldClose(m_window, true);
    postOtherEvent(m_window, WndEventType_WindowClose);
    return NO;
}

-(void)mouseDown:(NSEvent *)event {
    postButtonEvent(m_window, WndEventType_MouseDown, event);
}

-(void)mouseUp:(NSEvent *)event {
    postButtonEvent(m_window, WndEventType_MouseUp, event);
}

-(void)mouseDragged:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WndEventType_MouseMotion, event);
}

-(void)scrollWheel:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WndEventType_MouseScroll, event);
}

-(void)rightMouseDown:(NSEvent *)event {
    postButtonEvent(m_window, WndEventType_MouseDown, event);
}

-(void)rightMouseUp:(NSEvent *)event {
    postButtonEvent(m_window, WndEventType_MouseUp, event);
}

-(void)rightMouseDragged:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WndEventType_MouseMotion, event);
}

-(void)otherMouseDown:(NSEvent *)event {
    postButtonEvent(m_window, WndEventType_MouseDown, event);
}

-(void)otherMouseUp:(NSEvent *)event {
    postButtonEvent(m_window, WndEventType_MouseUp, event);
}

-(void)otherMouseDragged:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WndEventType_MouseMotion, event);
}

-(void)mouseMoved:(NSEvent *)event {
    postMotionOrScrollEvent(m_window, WndEventType_MouseMotion, event);
}

-(void)keyDown:(NSEvent *)event {
    postKeyEvent(m_window, WndEventType_KeyDown, event);
}

-(void)keyUp:(NSEvent *)event {
    postKeyEvent(m_window, WndEventType_KeyUp, event);
}

@end

///////////////////////////////////////////////////////////////////////////////
// Platform functions /////////////////////////////////////////////////////////

bool platform_init(void) {
    @autoreleasepool {

    [NSApplication sharedApplication];

    _wnd.delegate = [[AppDelegate alloc] init];
    if (!_wnd.delegate) {
        error("Failed to initialize _wnd.delegate");
        return false;
    }

    [NSApp setDelegate:_wnd.delegate];
    [NSApp finishLaunching];

    _wnd.bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"));
    if (!_wnd.bundle) {
        error("Failed to locate OpenGL framework");
        return false;
    }

    return true;

    } // autoreleasepool
}

void platform_fini(void) {
    @autoreleasepool {

    [NSApp terminate:nil];
    [_wnd.delegate release];

    } // autoreleasepool
}

bool window_init(window_t *window) {
    @autoreleasepool {

    if (!window)
        return false;

    if (!window->title) window->title   = "";
    if (!window->w)     window->w       = 640;
    if (!window->h)     window->h       = 480;

    window_cocoa_t *native = calloc(1, sizeof *native);

    if (!native) {
        error("Failed to allocate native window");
        return false;
    }

    native->window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, window->w, window->h)
        styleMask:          NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable
        backing:            NSBackingStoreBuffered
        defer:              NO
    ];

    if (!native->window) {
        error("Failed to create NSWindow");
        return false;
    }

    [native->window setAcceptsMouseMovedEvents:YES];
    [native->window setRestorable:NO];
    [native->window center];
    [native->window makeKeyAndOrderFront:nil];
    [native->window orderFront:nil];

    native->view = [[view_t alloc] initWithFrame:NSMakeRect(0, 0, window->w, window->h) window:native];

    if (!native->view) {
        error("Failed to create native->view");
        return false;
    }

    [native->window setContentView:native->view];
    [native->window setDelegate:native->view];
    [native->window makeFirstResponder:native->view];

    return true;

    } // autoreleasepool
}

void window_fini(window_t window) {
    @autoreleasepool {

    window_cocoa_t *native = window.native;

    if (!native)
        return;

    if (native->window) {
        if (native->view) {
            [native->view release];
            native->view = nil;
        }
        [native->window setDelegate:nil];
        [native->window close];
        native->window = nil;
    }

    } // autoreleasepool
}

void window_make_current(window_t window) {
    @autoreleasepool {

    [[((window_cocoa_t *)native)->view openGLContext] makeCurrentContext];

    } // autoreleasepool
}

void window_swap_buffers(window_t window) {
    @autoreleasepool {

    [[((window_cocoa_t *)native)->view openGLContext] flushBuffer];

    } // autoreleasepool
}

bool window_poll_events(window_callback_t *callback) {
    @autoreleasepool {

    while (true) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (!event)
            break;

        switch ([event type]) {
            case NSEventTypeKeyDown:
            case NSEventTypeKeyUp: {
                _wnd.closed = callback([event type] == NSEventTypeKeyDown ? EVENTTYPE_KEYDOWN : EVENTTYPE_KEYUP, &(window_event_t){
                    .key.code = [event keyCode],
                    .key.sym  = [event keyCode],
                    .key.mods = [event modifierFlags],
                    .key.x    = [event locationInWindow].x,
                    .key.y    = [event locationInWindow].y,
                });
            } break;

            // TODO: Others

            default: {
                [NSApp sendEvent:event];
            } break;
        }
    }

    return _wnd.closed;

    } // autoreleasepool
}

void window_set_pos(window_t *window, int x, int y) {
    @autoreleasepool {

    window_cocoa_t *native = window->native;
    NSRect rect  = NSMakeRect(x, translateYCoordinate(y + [native->view frame].size.height - 1), 0, 0);
    NSRect frame = [native->window frameRectForContentRect:rect];
    [native->window setFrameOrigin:frame.origin];

    window->x = x;
    window->y = y;

    } // autoreleasepool
}

void window_set_size(window_t *window, int w, int h) {
    @autoreleasepool {

    window_cocoa_t *native = window.native;
    NSRect contentRect = [native->window contentRectForFrameRect:[native->window frame]];
    contentRect.origin.y += contentRect.size.height - h;
    contentRect.size = NSMakeSize(w, h);
    [native->window setFrame:[native->window frameRectForContentRect:contentRect] display:YES];

    window->w = w;
    window->h = h;

    } // autoreleasepool
}

void window_set_title(WndWindow *window, char const *title) {
    @autoreleasepool {

    window_cocoa_t *native = window.native;
    [native->window setTitle:@(title)];

    window->title = title;

    } // autoreleasepool
}

gl_proc_t *window_proc_address(char const *name) {
    CFStringRef symbol_name = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingASCII);
    gl_proc_t *symbol = CFBundleGetFunctionPointerForName(_wnd.bundle, symbol_name);
    CFRelease(symbol_name);
    return symbol;
}
