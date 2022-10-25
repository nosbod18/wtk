sources  := source/wtk.c
includes := source source/plugins/log
INCLUDES := include
cflags   := -std=c99 -Wall

ifeq ($(OS),Darwin)
	sources += source/platform/cocoa.m
	cflags  += -DWTK_USE_COCOA -Wno-deprecated-declarations
	LDFLAGS := -framework Cocoa -framework OpenGL
else ifeq ($(OS),Linux)
	sources += source/platform/x11.c
	cflags  += -DWTK_USE_X11
	LDFLAGS := -lX11 -lGL
else
	$(error "Wtk currenly only supports MacOS and Linux")
endif