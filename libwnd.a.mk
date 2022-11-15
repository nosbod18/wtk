sources  := source/wnd.c
includes := source source/plugins/log
INCLUDES := include
cflags   := -std=c99 -Wall

ifeq ($(OS),Darwin)
	sources += source/platform/cocoa.m
	cflags  += -Wno-deprecated-declarations
	LDFLAGS := -framework Cocoa -framework OpenGL
else ifeq ($(OS),Linux)
	sources += source/platform/x11.c
	LDFLAGS := -lX11 -lGL
else
	$(error "Wnd currently only supports MacOS and Linux")
endif
