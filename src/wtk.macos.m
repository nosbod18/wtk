#define GL_SILENCE_DEPRECATION
#import "wtk.h"
#import <Cocoa/Cocoa.h>

@interface cocoa_view_t : NSOpenGLView <NSWindowDelegate>
@end

static struct {
    int nwindows;
} _wtk = {0};

///////////////////////////////////////////////////////////////////////////////
// Helper functions

static float translateYCoordinate(float y) {
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}

static void default_event_callback(wtk_window_t *window, int type, wtk_event_t const *event) {
    (void)window; (void)type; (void)event;
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

@implementation cocoa_view_t

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

    self = [super initWithFrame:frame pixelFormat:[[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease]];
    return self;
}

-(BOOL)windowShouldClose:(NSNotification *)notification {
    return NO;
}

@end

///////////////////////////////////////////////////////////////////////////////
// Platform functions /////////////////////////////////////////////////////////

bool start_plaform(void) {
    @autoreleasepool {

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    create_menubar();
    [NSApp finishLaunching];

    return 1;

    } // autoreleasepool
}

void stop_platform(void) {
    @autoreleasepool {

    [NSApp terminate:nil];

    } // autoreleasepool
}

int wtk_open_window(wtk_window_t *window) {
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

    cocoa_view_t *view = [[cocoa_view_t alloc] initWithFrame:NSMakeRect(0, 0, window->w, window->h)];
    if (!view)
        return 0;

    [native setContentView:view];
    [native setDelegate:view];
    [native makeFirstResponder:view];

    return 1;

    } // autoreleasepool
}

void wtk_close_window(wtk_window_t window) {
    @autoreleasepool {

    NSWindow *native = window.native;
    if (!native)
        return;

    if (native) {
        [[native contentView] release];
        [native setDelegate:nil];
        [native close];
        native = nil;
    }

    _wtk.nwindows--;
    if (_wtk.nwindows == 0)
        stop_platform();

    } // autoreleasepool
}

void wtk_move_window(wtk_window_t *window, int x, int y) {
    @autoreleasepool {

    NSWindow *native = window->native;
    NSRect rect  = NSMakeRect(x, translateYCoordinate(y + [[native contentView] frame].size.height - 1), 0, 0);
    NSRect frame = [native frameRectForContentRect:rect];
    [native setFrameOrigin:frame.origin];

    window->x = x;
    window->y = y;

    } // autoreleasepool
}

void wtk_resize_window(wtk_window_t *window, int w, int h) {
    @autoreleasepool {

    NSWindow *native = window->native;
    NSRect contentRect = [native contentRectForFrameRect:[native frame]];
    contentRect.origin.y += contentRect.size.height - h;
    contentRect.size = NSMakeSize(w, h);
    [native setFrame:[native frameRectForContentRect:contentRect] display:YES];

    window->w = w;
    window->h = h;

    } // autoreleasepool
}

void wtk_rename_window(wtk_window_t *window, char const *title) {
    @autoreleasepool {

    NSWindow *native = window->native;
    [native setTitle:@(title)];

    window->title = title;

    } // autoreleasepool
}

void wtk_make_current(wtk_window_t window) {
    @autoreleasepool {

    [[[(NSWindow *)window.native contentView] openGLContext] makeCurrentContext];

    } // autoreleasepool
}

void wtk_swap_buffers(wtk_window_t window) {
    @autoreleasepool {

    [[[(NSWindow *)window.native contentView] openGLContext] flushBuffer];

    } // autoreleasepool
}

void wtk_poll_events(wtk_window_t *window) {
    @autoreleasepool {

    while (1) {
        NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        if (!event)
            break;

        switch ([event type]) {
            case NSEventTypeKeyDown:
            case NSEventTypeKeyUp: {
                window->callback(window, [event type] == NSEventTypeKeyDown ? WTK_EVENT_KEYDOWN : WTK_EVENT_KEYUP, &(wtk_event_t){
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

