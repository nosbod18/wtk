sources  := src/wtk/wtk.c
INCLUDES := include
includes := src
cflags   := -Wall -Wextra -std=c99

ifeq ($(OS),macos)
	sources += src/platform/cocoa.m
	cflags  += -DWTK_COCOA -x objective-c -Wno-deprecated-declarations
	LDFLAGS := -framework Cocoa -framework OpenGL
else ifeq ($(OS),linux)
	sources += src/platform/x11.c
	cflags  += -DWTK_X11
	LDFLAGS := -lX11 -lGL
else
	$(error "Wtk only supports MacOS and Linux")
endif