SOURCES := 02-events.c
INCLUDE := ../include
DEPENDS := libwtk.a

ifeq ($(OS),Darwin)
	LDFLAGS := -framework Cocoa -framework OpenGL
else ifeq ($(OS),Linux)
	LDFLAGS := -lX11 -lGL
endif