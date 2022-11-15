#pragma once

typedef enum WndEventType {
    WndEventType_WindowFocusOut,
    WndEventType_WindowFocusIn,
    WndEventType_WindowResize,
    WndEventType_WindowClose,
    WndEventType_MouseMotion,
    WndEventType_MouseScroll,
    WndEventType_MouseEnter,
    WndEventType_MouseLeave,
    WndEventType_MouseDown,
    WndEventType_MouseUp,
    WndEventType_KeyDown,
    WndEventType_KeyUp,
} WndEventType;

typedef enum WndKey {
    WndKey_Backspace = 0x08, WndKey_Tab = 0x09, WndKey_Enter = 0x0a, WndKey_Escape = 0x1b,

    // Ascii keys can use their character representation, e.g. 'w' for the W key

    WndKey_Up = 0x80, WndKey_Down, WndKey_Left, WndKey_Right,
    WndKey_F1, WndKey_F2, WndKey_F3, WndKey_F4, WndKey_F5, WndKey_F6, WndKey_F7, WndKey_F8, WndKey_F9, WndKey_F10, WndKey_F11, WndKey_F12,
    WndKey_LeftShift, WndKey_LeftControl, WndKey_LeftSuper, WndKey_LeftAlt,
    WndKey_RightShift, WndKey_RightControl, WndKey_RightSuper, WndKey_RightAlt,
    WndKey_CapsLock,
} WndKey;

typedef enum WndButton {
    WndButton_1,
    WndButton_2,
    WndButton_3,
    WndButton_4,
    WndButton_5,
    WndButton_6,
    WndButton_7,
    WndButton_8,
} WndButton;

typedef enum WndMod {
    WndMod_Shift        = 1U << 0,
    WndMod_Control      = 1U << 1,
    WndMod_Alt          = 1U << 2,
    WndMod_Super        = 1U << 3,
} WndMod;

typedef struct WndWindow WndWindow;

typedef struct WndEvent {
    WndWindow              *window;
    WndEventType            type;
    double                  time;

    struct { int x, y; }    location;
    struct { int x, y, z; } delta;

    WndKey                  key;
    WndButton               button;
    int                     sym;
    unsigned int            mods;
    int                     isRepeat;
    int                     clicks;
} WndEvent;

typedef void WndEventCallback(WndEvent const *event);
typedef int  WndErrorCallback(char const *fmt, ...);

typedef struct WndWindowDesc {      // Defaults:
    struct {
        int major;                  // cocoa: 3, x11: 4
        int minor;                  // cocoa: 2, x11: 6
    } context;

    WndEventCallback *onEvent;      // stub function, does nothing
    char const *title;              // "Untitled"
    int width;                      // 640
    int height;                     // 480
} WndWindowDesc;

typedef void WndGLProc(void);

WndWindow      *WndCreateWindow         (WndWindowDesc *desc);
void            WndDeleteWindow         (WndWindow *window);

void            WndMakeCurrent          (WndWindow const *window);
void            WndSwapBuffers          (WndWindow const *window);
void            WndPollEvents           (void);

void            WndGetWindowPos         (WndWindow const *window, int *x, int *y);
void            WndGetWindowSize        (WndWindow const *window, int *w, int *h);
int             WndGetWindowShouldClose (WndWindow const *window);

void            WndSetWindowPos         (WndWindow *window, int x, int y);
void            WndSetWindowSize        (WndWindow *window, int w, int h);
void            WndSetWindowTitle       (WndWindow *window, char const *title);
void            WndSetWindowShouldClose (WndWindow *window, int shouldClose);

WndGLProc      *WndGetProcAddress       (char const *name);
