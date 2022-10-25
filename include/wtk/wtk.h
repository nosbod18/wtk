#pragma once

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

typedef struct WtkEvent {
    WtkWindow              *window;
    WtkEventType            type;
    double                  time;

    struct { int x, y; }    location;
    struct { int x, y, z; } delta;

    WtkKey                  key;
    WtkButton               button;
    int                     sym;
    unsigned int            mods;
    int                     isRepeat;
    int                     clicks;
} WtkEvent;

typedef void WtkEventCallback(WtkEvent const *event);
typedef int  WtkErrorCallback(char const *fmt, ...);

typedef struct WtkWindowDesc {      // Defaults:
    struct {
        int major;                  // cocoa: 3, x11: 4
        int minor;                  // cocoa: 2, x11: 6
    } context;

    WtkEventCallback *onEvent;      // stub function, does nothing
    char const *title;              // "Untitled"
    int width;                      // 640
    int height;                     // 480
} WtkWindowDesc;

typedef void WtkGLProc(void);

WtkWindow      *wtkCreateWindow         (WtkWindowDesc *desc);
void            wtkDeleteWindow         (WtkWindow *window);

void            wtkMakeCurrent          (WtkWindow const *window);
void            wtkSwapBuffers          (WtkWindow const *window);
void            wtkPollEvents           (void);

void            wtkGetWindowPos         (WtkWindow const *window, int *x, int *y);
void            wtkGetWindowSize        (WtkWindow const *window, int *w, int *h);
int             wtkGetWindowShouldClose (WtkWindow const *window);

void            wtkSetWindowPos         (WtkWindow *window, int x, int y);
void            wtkSetWindowSize        (WtkWindow *window, int w, int h);
void            wtkSetWindowTitle       (WtkWindow *window, char const *title);
void            wtkSetWindowShouldClose (WtkWindow *window, int shouldClose);

WtkGLProc      *wtkGetProcAddress       (char const *name);