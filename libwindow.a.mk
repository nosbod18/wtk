sources := source/$(OS).?
cflags  := -std=c99 -Wall -Wextra -Wpedantic

ifeq ($(OS),macos)
	cflags  += -Wno-deprecated-declarations
	LDFLAGS := -framework Cocoa -framework OpenGL
else ifeq ($(OS),linux)
	LDFLAGS := -lX11 -lGL
else
	$(error "Wnd currently only supports MacOS and Linux")
endif
