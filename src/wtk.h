#pragma once

///////////////////////////////////////////////////////////////////////////////
// Enums

enum {
    WTK_EVENT_CLOSE,
    WTK_EVENT_RESIZE,
    WTK_EVENT_FOCUSIN,
    WTK_EVENT_FOCUSOUT,
    WTK_EVENT_KEYUP,
    WTK_EVENT_KEYDOWN,
    WTK_EVENT_MOUSEUP,
    WTK_EVENT_MOUSEDOWN,
    WTK_EVENT_MOUSEENTER,
    WTK_EVENT_MOUSELEAVE,
    WTK_EVENT_MOUSESCROLL,
    WTK_EVENT_MOUSEMOTION,
};

enum {
    WTK_KEY_BACKSPACE = 0x08, WTK_KEY_TAB = 0x09, WTK_KEY_ENTER = 0x0a, WTK_KEY_ESCAPE = 0x1b,

    // TODO: Ascii keys

    WTK_KEY_UP = 0x80, WTK_KEY_DOWN, WTK_KEY_LEFT, WTK_KEY_RIGHT,
    WTK_KEY_F1, WTK_KEY_F2, WTK_KEY_F3, WTK_KEY_F4, WTK_KEY_F5, WTK_KEY_F6, WTK_KEY_F7, WTK_KEY_F8, WTK_KEY_F9, WTK_KEY_F10, WTK_KEY_F11, WTK_KEY_F12,
    WTK_KEY_LSHIFT, WTK_KEY_LCTRL, WTK_KEY_LSUPER, WTK_KEY_LALT,
    WTK_KEY_RSHIFT, WTK_KEY_RCTRL, WTK_KEY_RSUPER, WTK_KEY_RALT,
    WTK_KEY_CAPSLOCK,
};

enum {
    WTK_MOUSE_1, WTK_MOUSE_2, WTK_MOUSE_3, WTK_MOUSE_4, WTK_MOUSE_5, WTK_MOUSE_6, WTK_MOUSE_7, WTK_MOUSE_8,
};

enum {
    WTK_MOD_SHIFT   = 1U << 0,
    WTK_MOD_CTRL    = 1U << 1,
    WTK_MOD_ALT     = 1U << 2,
    WTK_MOD_SUPER   = 1U << 3,
};

///////////////////////////////////////////////////////////////////////////////
// Structs

typedef union wtk_event_t {
    struct { int dx, dy;                } scroll, motion;
    struct { int code, sym, mods, x, y; } key, button;
} wtk_event_t;

typedef struct wtk_window_t {
    void (*callback)(struct wtk_window_t *window, int type, wtk_event_t const *event);
    char const *title;
    int x, y, w, h;
    int closed;
    void *native;
} wtk_window_t;

///////////////////////////////////////////////////////////////////////////////
// Prototypes

int         wtk_open_window     (wtk_window_t *window);
void        wtk_close_window    (wtk_window_t  window);

void        wtk_move_window     (wtk_window_t *window, int x, int y);
void        wtk_resize_window   (wtk_window_t *window, int w, int h);
void        wtk_rename_window   (wtk_window_t *window, char const *title);

void        wtk_make_current    (wtk_window_t  window);
void        wtk_swap_buffers    (wtk_window_t  window);
void        wtk_poll_events     (wtk_window_t *window);

