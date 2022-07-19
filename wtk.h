#pragma once
#include <stdbool.h>

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

typedef union WtkEventData {
    struct { int width, height;     } resize;
    struct { int key, sym, mods;    } key;
    struct { int button, mods, x, y;} button, motion;
    struct { int dx, dy;            } scroll;
} WtkEventData;

typedef struct WtkWindow WtkWindow;

typedef void WtkEventCallback(WtkWindow *window, WtkEventType type, WtkEventData const *data);

typedef struct WtkWindowDesc {
    WtkEventCallback   *callback;
    char const         *title;
    int                 width;
    int                 height;
    bool                fullscreen;
    bool                hidden;
    bool                vsync;
} WtkWindowDesc;


WtkWindow  *wtkCreateWindow         (WtkWindowDesc const *desc);
void        wtkDeleteWindow         (WtkWindow *window);
void        wtkSwapBuffers          (WtkWindow const *window);
void        wtkPollEvents           (void);

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

