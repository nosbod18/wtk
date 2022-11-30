#define GL_SILENCE_DEPRECATION
#import "../window.h"
#import <Cocoa/Cocoa.h>

@interface view_t : NSOpenGLView <NSWindowDelegate>
@end

static struct {
    CFBundleRef bundle;
    int nwindows;
} _wnd = {0};

///////////////////////////////////////////////////////////////////////////////
// Helper functions

static float translateYCoordinate(float y) {
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}

///////////////////////////////////////////////////////////////////////////////
// Class functions

static void create_menubar(void) {
    @autoreleasepool {

    NSMenu *menu_bar, *app_menu;
    NSMenuItem *app_menu_item, *quit_menu_item;
    NSString *app_name, *quit_title;

    menu_bar = [[[NSMenu alloc] init] autorelease];
    [NSApp setMainMenu:menu_bar];

    app_menu_item = [[[NSMenuItem alloc] init] autorelease];
    [menu_bar addItem:app_menu_item];

    app_menu = [[[NSMenu alloc] init] autorelease];
    [app_menu_item setSubmenu:app_menu];

    app_name = [[NSProcessInfo processInfo] processName];
    quit_title = [@"Quit " stringByAppendingString:app_name];
    quit_menu_item = [[[NSMenuItem alloc] initWithTitle:quit_title
                                                 action:@selector(terminate:)
                                          keyEquivalent:@"q"] autorelease];
    [app_menu addItem:quit_menu_item];

    }
}

@implementation view_t

- (id)initWithFrame:(NSRect)frame  {
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

-(BOOL)windowShouldClose:(NSNotification *)notification {
    return NO;
}

@end

///////////////////////////////////////////////////////////////////////////////
// Platform functions /////////////////////////////////////////////////////////

bool platform_start(void) {
    @autoreleasepool {

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    create_menubar();
    [NSApp finishLaunching];

    if (!(_wnd.bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.opengl"))))
        return 0;

    return 1;

    } // autoreleasepool
}

void platform_stop(void) {
    @autoreleasepool {

    [NSApp terminate:nil];

    } // autoreleasepool
}

int window_init(window_t *window) {
    @autoreleasepool {

    if (!window)
        return 0;

    if (!window->callback)  window->callback = default_event_callback;
    if (!window->title)     window->title = "";
    if (!window->w)         window->w = 640;
    if (!window->h)         window->h = 480;

    NSWindow *native = [[NSWindow alloc]
                            initWithContentRect:NSMakeRect(0, 0, window->w, window->h)
                                      styleMask:NSWindowStyleMaskMiniaturizable|NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable
                                        backing:NSBackingStoreBuffered
                                          defer:NO];
    if (!native)
        return 0;

    [native setAcceptsMouseMovedEvents:YES];
    [native setRestorable:NO];
    [native center];
    [native makeKeyAndOrderFront:nil];
    [native orderFront:nil];

    view_t *view = [[view_t alloc] initWithFrame:NSMakeRect(0, 0, window->w, window->h)];
    if (!view)
        return 0;

    [native->window setContentView:native->view];
    [native->window setDelegate:native->view];
    [native->window makeFirstResponder:native->view];

    return 1;

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

void window_poll_events(window_t *window) {
    @autoreleasepool {

    while (1) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (!event)
            break;

        switch ([event type]) {
            case NSEventTypeKeyDown:
            case NSEventTypeKeyUp: {
                window->callback(window, [event type] == NSEventTypeKeyDown ? EVENTTYPE_KEYDOWN : EVENTTYPE_KEYUP, &(window_event_t){
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
