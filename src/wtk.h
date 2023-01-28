#pragma once
#include <stdbool.h>

enum {
    WTK_EVENTTYPE_KEYUP,
    WTK_EVENTTYPE_KEYDOWN,
    WTK_EVENTTYPE_MOUSEUP,
    WTK_EVENTTYPE_MOUSEDOWN,
    WTK_EVENTTYPE_MOUSEENTER,
    WTK_EVENTTYPE_MOUSELEAVE,
    WTK_EVENTTYPE_MOUSESCROLL,
    WTK_EVENTTYPE_MOUSEMOTION,
    WTK_EVENTTYPE_WINDOWCLOSE,
    WTK_EVENTTYPE_WINDOWRESIZE,
    WTK_EVENTTYPE_WINDOWFOCUSIN,
    WTK_EVENTTYPE_WINDOWFOCUSOUT,
};

// Ascii keys can use their character representation, e.g. 'w', 'A', '+', ...
enum {
    WTK_KEY_BACKSPACE = 0x08, WTK_KEY_TAB = 0x09, WTK_KEY_ENTER = 0x0a, WTK_KEY_ESCAPE = 0x1b,
    WTK_KEY_UP = 0x80, WTK_KEY_DOWN, WTK_KEY_LEFT, WTK_KEY_RIGHT,
    WTK_KEY_PAGEUP, WTK_KEY_PAGEDOWN, WTK_KEY_HOME, WTK_KEY_END, WTK_KEY_INSERT, WTK_KEY_DELETE,
    WTK_KEY_F1, WTK_KEY_F2, WTK_KEY_F3, WTK_KEY_F4, WTK_KEY_F5, WTK_KEY_F6, WTK_KEY_F7, WTK_KEY_F8, WTK_KEY_F9, WTK_KEY_F10, WTK_KEY_F11, WTK_KEY_F12,
    WTK_KEY_LSHIFT, WTK_KEY_LCTRL, WTK_KEY_LSUPER, WTK_KEY_LALT,
    WTK_KEY_RSHIFT, WTK_KEY_RCTRL, WTK_KEY_RSUPER, WTK_KEY_RALT,
    WTK_KEY_CAPSLOCK,
};

enum {
    WTK_BUTTON_1,
    WTK_BUTTON_2,
    WTK_BUTTON_3,
    WTK_BUTTON_4,
    WTK_BUTTON_5,
    WTK_BUTTON_6,
    WTK_BUTTON_7,
    WTK_BUTTON_8,
};

enum {
    WTK_MOD_SHIFT    = 0x01,
    WTK_MOD_CTRL     = 0x02,
    WTK_MOD_ALT      = 0x04,
    WTK_MOD_SUPER    = 0x08,
    WTK_MOD_CAPSLOCK = 0x10,
};

typedef struct WtkEvent {
    int type;
    int keyCode;
    int buttonNumber;
    unsigned int modifiers;
    struct { int x, y; } location, delta;
} WtkEvent;

typedef struct WtkWindow WtkWindow;

typedef struct WtkWindowDesc {
    void (*on_event)(WtkWindow *window, WtkEvent const *event);
    char const *title;
    int x, y, w, h;
} WtkWindowDesc;

WtkWindow  *WtkCreateWindow         (WtkWindowDesc const *desc);
void        WtkMakeCurrent          (WtkWindow *window);
void        WtkSwapBuffers          (WtkWindow *window);
void        WtkPollEvents           (void);
void        WtkDeleteWindow         (WtkWindow *window);

void        WtkGetWindowRect        (WtkWindow const *window, int *x, int *y, int *w, int *h);
bool        WtkGetWindowShouldClose (WtkWindow const *window);

void        WtkSetWindowOrigin      (WtkWindow *window, int x, int y);
void        WtkSetWindowSize        (WtkWindow *window, int w, int h);
void        WtkSetWindowTitle       (WtkWindow *window, char const *title);
void        WtkSetWindowShouldClose (WtkWindow *window, bool should_close);
