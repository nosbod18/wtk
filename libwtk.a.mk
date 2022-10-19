SOURCES := source/wtk.c
INCLUDE := include source
CFLAGS  := -std=c99 -Wall

ifeq ($(OS),Darwin)
	CFLAGS += -DWTK_USE_COCOA -x objective-c -Wno-deprecated-declarations
else ifeq ($(OS),Linux)
	CFLAGS += -DWTK_USE_X11
else
	$(error "Wtk currenly only supports MacOS and Linux")
endif