MAKEFLAGS += -r
.SUFFIXES:
.SECONDEXPANSION:
.RECIPEPREFIX := >

OS        ?= $(shell uname -s)
ARCH      ?= $(shell uname -m)
MODE      ?= debug
BUILD     ?= build/$(OS)-$(ARCH)-$(MODE)/
MKEXT     ?= mk

compilerof = $(if $(filter-out %.c.o %.m.o,$(filter %.o,$1)),CXX,CC)
canonical  = $(patsubst $(CURDIR)/%,%,$(abspath $1))
flagsof    = $1.$(if $(filter-out %.c.o %.m.o,$(filter %.o,$1)),CXX,C)FLAGS
print      = $(if $V,$(strip $2),$(if $Q,@$2,@$(if $1,printf $1;) $2))

define add.mk
    SOURCES  := # Project source files, supports wildcards, required
    INCLUDE  := # Directories to use as include path for compiler (-I...), supports wildcards
    DEPENDS  := # Subprojects this project depends on (automatically links any libraries)
    CFLAGS   := # C compiler flags
    CXXFLAGS := # CXX compiler flags
    LDFLAGS  := # Linker flags

    include $1

    $$(if $$(strip $$(SOURCES)),,$$(error $1: SOURCES cannot be empty))

    name := $$(patsubst %.$(MKEXT),%,$$(notdir $1))
    P := $(BUILD)$$(if $$(suffix $$(name)),lib,bin)/$$(name)

    $$P.SOURCES := $$(wildcard $$(call canonical,$$(SOURCES:%=$$(dir $1)%)))
    $$P.INCLUDE := $$(wildcard $$(call canonical,$$(INCLUDE:%=$$(dir $1)%)))
    $$P.OBJECTS := $$($$P.SOURCES:%=$$(BUILD)obj/%.o)
    $$P.DEPENDS := $$(foreach d,$$(DEPENDS),$(BUILD)$$(if $$(suffix $$d),lib,bin)/$$d) $1
    $$P.LDFLAGS := $$(LDFLAGS) $$(filter %.a,$$($$P.DEPENDS))

    $$(foreach o,$$($$P.OBJECTS),\
        $$(eval $$o.CFLAGS := $$(CFLAGS))\
        $$(eval $$o.CXXFLAGS := $$(CXXFLAGS))\
        $$(eval $$(call flagsof,$$o) += $$($$P.INCLUDE:%=-I%)))

    products += $$P
    files += $$P $$($$P.OBJECTS)
endef

define add.o
    $1.DEPENDS := $(1:$(BUILD)obj/%.o=%)
    $1.MESSAGE := "%-3s $1\n" "$$(call compilerof,$1)"
    $1.COMMAND := $$($$(call compilerof,$1)) $$($$(call flagsof,$1)) -MMD -MP -c -o $1 $$($1.DEPENDS)
endef

define add.a
    $1.DEPENDS += $$($1.OBJECTS)
    $1.MESSAGE := "AR  $1\n"
    $1.COMMAND := $(AR) rcs $1 $$($1.OBJECTS)
endef

define add
    $1.DEPENDS += $$($1.OBJECTS)
    $1.MESSAGE := "LD  $1\n"
    $1.COMMAND := $$($$(call compilerof,$$($1.DEPENDS))) -o $1 $$($1.OBJECTS) $$($1.LDFLAGS)
endef

ifneq ($(MAKECOMMANDGOALS),clean)
    modules  := $(shell find . -name '*.$(MKEXT)' | cut -c3-) # cut -c3- removes the leading ./
    products :=
    files    :=
    $(foreach m,$(modules),$(eval $(call add.mk,$m)))
    $(foreach f,$(files),$(eval $(call add$(suffix $f),$f)))
endif

all: $(products)
.PHONY: all

clean:
> $(call print,"RM  $(BUILD)\n",rm -rf $(BUILD))
.PHONY: clean

$(files): $$($$@.DEPENDS)
> $(call print,,mkdir -p $(@D))
> $(call print,$($@.MESSAGE),$($@.COMMAND))

ifneq ($(MAKECOMMANDGOALS),clean)
    -include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
endif
