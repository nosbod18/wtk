#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef enum WtkEventType {
    WtkEventType_WindowClose,
    WtkEventType_WindowResize,
    WtkEventType_WindowFocus,
    WtkEventType_WindowUnfocus,
    WtkEventType_MouseMotion,
    WtkEventType_MouseScroll,
    WtkEventType_MouseEnter,
    WtkEventType_MouseLeave,
    WtkEventType_MouseDown,
    WtkEventType_MouseUp,
    WtkEventType_KeyDown,
    WtkEventType_KeyUp,
} WtkEventType;

typedef enum WtkWindowFlags {
    WtkWindowFlags_Closable     = 1U << 0,
    WtkWindowFlags_Resizable    = 1U << 2,
    WtkWindowFlags_Centered     = 1U << 3,
    WtkWindowFlags_Titled       = 1U << 4,

    WtkWindowFlags_Visible      = 1U << 5,
    WtkWindowFlags_Fullscreen   = 1U << 6,
    WtkWindowFlags_Vsync        = 1U << 7,
} WtkWindowFlags;

typedef union WtkEventData {
    struct { int width, height;             } resize;
    struct { int keycode, scancode, mods;   } key;
    struct { int button, mods, x, y;        } button, motion;
    struct { int dx, dy;                    } scroll;
} WtkEventData;

typedef struct WtkWindow WtkWindow;

typedef void WtkEventCallback(WtkWindow *window, WtkEventType type, WtkEventData const *data);
typedef int  WtkErrorCallback(char const *fmt, ...);

typedef struct WtkWindowDesc {      // Defaults:
    struct {
        struct {
            int red;                // 8
            int green;              // 8
            int blue;               // 8
            int alpha;              // 8
            int depth;              // 24
            int stencil;            // 8
        } bits;
        int samples;                // 0
        int sampleBuffers;          // 0
    } context;

    WtkEventCallback *onEvent;      // defaultOnEvent (Internal to each backend, does nothing)
    char const *title;              // "Untitled"
    int width;                      // 640
    int height;                     // 480
    WtkWindowFlags flags;           // WtkWindowFlags_Closable | WtkWindowFlags_Resizable | WtkWindowFlags_Titled | WtkWindowFlags_Centered
} WtkWindowDesc;

typedef struct WtkDesc {            // Defaults:
    WtkErrorCallback *onError;      // printf
    void *(*alloc)(size_t size);    // malloc
    void  (*free)(void *ptr);       // free
} WtkDesc;


bool        wtkInit                 (WtkDesc *desc);
void        wtkQuit                 (void);
void        wtkMakeCurrent          (WtkWindow const *window);
void        wtkSwapBuffers          (WtkWindow const *window);
void        wtkPollEvents           (void);

WtkWindow  *wtkCreateWindow         (WtkWindowDesc *desc);
void        wtkDeleteWindow         (WtkWindow *window);

void        wtkGetWindowPos         (WtkWindow const *window, int *x, int *y);
void        wtkGetWindowSize        (WtkWindow const *window, int *width, int *height);
bool        wtkGetWindowVisible     (WtkWindow const *window);
bool        wtkGetWindowFullscreen  (WtkWindow const *window);
bool        wtkGetWindowShouldClose (WtkWindow const *window);

void        wtkSetWindowPos         (WtkWindow *window, int x, int y);
void        wtkSetWindowSize        (WtkWindow *window, int width, int height);
void        wtkSetWindowTitle       (WtkWindow *window, char const *title);
void        wtkSetWindowVisible     (WtkWindow *window, bool visible);
void        wtkSetWindowFullscreen  (WtkWindow *window, bool fullscreen);
void        wtkSetWindowShouldClose (WtkWindow *window, bool shouldClose);
