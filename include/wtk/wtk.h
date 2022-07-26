#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef enum WtkWindowFlag {
    WtkWindowFlag_Closable      = 1U << 0,
    WtkWindowFlag_Resizable     = 1U << 2,
    WtkWindowFlag_Centered      = 1U << 3,
    WtkWindowFlag_Titled        = 1U << 4,

    WtkWindowFlag_Visible       = 1U << 5,
    WtkWindowFlag_Fullscreen    = 1U << 6,
    WtkWindowFlag_Vsync         = 1U << 7,
} WtkWindowFlag;

typedef enum WtkEventType {
    WtkEventType_WindowFocusOut,
    WtkEventType_WindowFocusIn,
    WtkEventType_WindowResize,
    WtkEventType_WindowClose,
    WtkEventType_MouseMotion,
    WtkEventType_MouseScroll,
    WtkEventType_MouseEnter,
    WtkEventType_MouseLeave,
    WtkEventType_MouseDown,
    WtkEventType_MouseUp,
    WtkEventType_KeyDown,
    WtkEventType_KeyUp,
} WtkEventType;

typedef enum WtkKey {
    WtkKey_Backspace = 0x08, WtkKey_Tab = 0x09, WtkKey_Enter = 0x0a, WtkKey_Escape = 0x1b,

    // Ascii keys can use their character representation, e.g. 'w' for the W key

    WtkKey_Up = 0x80, WtkKey_Down, WtkKey_Left, WtkKey_Right,
    WtkKey_F1, WtkKey_F2, WtkKey_F3, WtkKey_F4, WtkKey_F5, WtkKey_F6, WtkKey_F7, WtkKey_F8, WtkKey_F9, WtkKey_F10, WtkKey_F11, WtkKey_F12,
    WtkKey_LeftShift, WtkKey_LeftControl, WtkKey_LeftSuper, WtkKey_LeftAlt,
    WtkKey_RightShift, WtkKey_RightControl, WtkKey_RightSuper, WtkKey_RightAlt,
    WtkKey_CapsLock,
} WtkKey;

typedef enum WtkButton {
    WtkButton_1,
    WtkButton_2,
    WtkButton_3,
    WtkButton_4,
    WtkButton_5,
    WtkButton_6,
    WtkButton_7,
    WtkButton_8,
} WtkButton;

typedef enum WtkMod {
    WtkMod_Shift        = 1U << 0,
    WtkMod_Control      = 1U << 1,
    WtkMod_Alt          = 1U << 2,
    WtkMod_Super        = 1U << 3,
} WtkMod;

typedef struct WtkWindow WtkWindow;

typedef union WtkEventData {
    struct { int width, height;             } resize;
    struct { int keycode, scancode, mods;   } key;
    struct { int button, mods, x, y;        } button, motion;
    struct { int dx, dy;                    } scroll;
} WtkEventData;

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
    WtkWindowFlag flags;            // WtkWindowFlag_Closable | WtkWindowFlag_Resizable | WtkWindowFlag_Titled | WtkWindowFlag_Centered | WtkWindowFlag_Visible
} WtkWindowDesc;

typedef struct WtkDesc {            // Defaults:
    WtkErrorCallback *onError;      // printf
    void *(*malloc)(size_t size);   // malloc
    void  (*free)(void *ptr);       // free
} WtkDesc;


bool            wtkInit                 (WtkDesc *desc);
void            wtkQuit                 (void);
void            wtkMakeCurrent          (WtkWindow const *window);
void            wtkSwapBuffers          (WtkWindow const *window);
void            wtkPollEvents           (void);

WtkWindow      *wtkCreateWindow         (WtkWindowDesc *desc);
void            wtkDeleteWindow         (WtkWindow *window);

void            wtkGetWindowPos         (WtkWindow const *window, int *x, int *y);
void            wtkGetWindowSize        (WtkWindow const *window, int *width, int *height);
WtkWindowFlag   wtkGetWindowFlags       (WtkWindow const *window, WtkWindowFlag flag);
bool            wtkGetWindowShouldClose (WtkWindow const *window);

void            wtkSetWindowPos         (WtkWindow *window, int x, int y);
void            wtkSetWindowSize        (WtkWindow *window, int width, int height);
void            wtkSetWindowFlag        (WtkWindow *window, WtkWindowFlag flag, bool set);
void            wtkSetWindowTitle       (WtkWindow *window, char const *title);
void            wtkSetWindowShouldClose (WtkWindow *window, bool shouldClose);
