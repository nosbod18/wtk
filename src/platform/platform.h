#include "wtk/wtk.h"
#include <stdbool.h>

#if defined(WTK_COCOA)

#import <Cocoa/Cocoa.h>

@interface CocoaView : NSOpenGLView <NSWindowDelegate>
- (id)initWithFrame:(NSRect)frame andWindow:(WtkWindow *)window;
@end

#elif defined(WTK_X11)

#include <X11/Xlib.h>
#include <GL/glx.h>

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
