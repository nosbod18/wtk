#pragma once

///////////////////////////////////////////////////////////////////////////////
// Enums

enum {
    WINDOWEVENT_CLOSE,
    WINDOWEVENT_RESIZE,
    WINDOWEVENT_FOCUSIN,
    WINDOWEVENT_FOCUSOUT,
    WINDOWEVENT_KEYUP,
    WINDOWEVENT_KEYDOWN,
    WINDOWEVENT_MOUSEUP,
    WINDOWEVENT_MOUSEDOWN,
    WINDOWEVENT_MOUSEENTER,
    WINDOWEVENT_MOUSELEAVE,
    WINDOWEVENT_MOUSESCROLL,
    WINDOWEVENT_MOUSEMOTION,
};

enum {
    KEY_BACKSPACE = 0x08, KEY_TAB = 0x09, KEY_ENTER = 0x0a, KEY_ESCAPE = 0x1b,

    // TODO: Ascii keys

    KEY_UP = 0x80, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_LEFTSHIFT,  KEY_LEFTCTRL,  KEY_LEFTSUPER,  KEY_LEFTALT,
    KEY_RIGHTSHIFT, KEY_RIGHTCTRL, KEY_RIGHTSUPER, KEY_RIGHTALT,
    KEY_CAPSLOCK,
};

enum {
    MOUSE_1, MOUSE_2, MOUSE_3, MOUSE_4, MOUSE_5, MOUSE_6, MOUSE_7, MOUSE_8,
};

enum {
    MOD_SHIFT        = 1U << 0,
    MOD_CONTROL      = 1U << 1,
    MOD_ALT          = 1U << 2,
    MOD_SUPER        = 1U << 3,
};

///////////////////////////////////////////////////////////////////////////////
// Structs

typedef union window_event_t {
    struct { int dx, dy;                } scroll, motion;
    struct { int code, sym, mods, x, y; } key, button;
} window_event_t;

typedef struct window_t {
    void (*callback)(struct window_t *window, int type, window_event_t const *event);
    char const *title;
    int x, y, w, h;
    int closed;
    void *native;
} window_t;

// TODO: Rename
typedef void gl_proc_t(void);

///////////////////////////////////////////////////////////////////////////////
// Prototypes

int         window_init         (window_t *window);
void        window_fini         (window_t  window);

void        window_make_current (window_t  window);
void        window_swap_buffers (window_t  window);
void        window_poll_events  (window_t *window);

void        window_move         (window_t *window, int x, int y);
void        window_resize       (window_t *window, int w, int h);
void        window_rename       (window_t *window, char const *title);

gl_proc_t  *window_proc_address (char const *name);
