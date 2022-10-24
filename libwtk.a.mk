sources  := source/wtk.c
includes := source
INCLUDES := include
cflags   := -std=c99 -Wall

ifeq ($(OS),Darwin)
	cflags  += -DWTK_USE_COCOA -x objective-c -Wno-deprecated-declarations
	LDFLAGS := -framework Cocoa -framework OpenGL
else ifeq ($(OS),Linux)
	cflags  += -DWTK_USE_X11
	LDFLAGS := -lX11 -lGL
else
	$(error "Wtk currenly only supports MacOS and Linux")
endif