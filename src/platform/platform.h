#include "wtk/wtk.h"
#include <stdbool.h>

#if defined(WTK_COCOA)

#import <Cocoa/Cocoa.h>

@interface CocoaApp : NSObject <NSApplicationDelegate>
@end

@interface CocoaView : NSOpenGLView <NSWindowDelegate>
- (id)initWithFrame:(NSRect)frame andWindow:(WtkWindow *)window;
@end

#elif defined(WTK_X11)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

typedef GLXContext GlXCreateContextAttribsARBProc(Display *, GLXFBConfig, GLXContext, Bool, int const *);

#endif // WTK_COCOA || WTK_X11

struct WtkWindow {
    WtkWindowDesc desc;
    bool closed;
#if defined(WTK_COCOA)
    NSWindow *window;
    CocoaView *view;
#elif defined(WTK_X11)
    Window window;
    GLXContext context;
#endif // WTK_COCOA || WTK_X11
};

static struct {
    int num_windows;
#if defined(WTK_COCOA)
    CocoaApp *app;
#elif defined(WTK_X11)
    GlXCreateContextAttribsARBProc *glx_create_ctx_attribs;
    Display *display;
    XContext context;
    Visual *visual;
    Window root;
    Colormap colormap;
    Atom wm_delwin;
    int screen;
    int depth;
#endif // WTK_COCOA || WTK_X11
} _wtk = {0};


bool platformInit            (void);
void platformTerminate       (void);
bool platformCreateWindow    (WtkWindow *window);
void platformMakeCurrent     (WtkWindow *window);
void platformSwapBuffers     (WtkWindow *window);
void platformPollEvents      (void);
void platformDeleteWindow    (WtkWindow *window);
void platformSetWindowOrigin (WtkWindow *window, int x, int y);
void platformSetWindowSize   (WtkWindow *window, int w, int h);
void platformSetWindowTitle  (WtkWindow *window, char const *title);