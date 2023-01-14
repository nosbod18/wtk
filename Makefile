############################################################################
#        nonrecmk build system - http://github.com/nosbod18/nonrecmk       #
############################################################################
#  This is free and unencumbered software released into the public domain. #
#                                                                          #
#  Anyone is free to copy, modify, publish, use, compile, sell, or         #
#  distribute this software, either in source code form or as a compiled   #
#  binary, for any purpose, commercial or non-commercial, and by any       #
#  means.                                                                  #
#                                                                          #
#  In jurisdictions that recognize copyright laws, the author or authors   #
#  of this software dedicate any and all copyright interest in the         #
#  software to the public domain. We make this dedication for the benefit  #
#  of the public at large and to the detriment of our heirs and            #
#  successors. We intend this dedication to be an overt act of             #
#  relinquishment in perpetuity of all present and future rights to this   #
#  software under copyright law.                                           #
#                                                                          #
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,         #
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF      #
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  #
#  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR       #
#  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   #
#  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR   #
#  OTHER DEALINGS IN THE SOFTWARE.                                         #
#                                                                          #
#  For more information, please refer to <http://unlicense.org/>           #
############################################################################

MAKEFLAGS += -r
SHELL := /bin/bash
.SUFFIXES:
.SECONDEXPANSION:
.RECIPEPREFIX := >

OS        := $(if $(OS),win32,$(subst darwin,macos,$(shell uname -s | tr [A-Z] [a-z])))
ARCH      ?= $(shell uname -m)
MODE      ?= debug
BUILD     ?= .build/$(OS)-$(ARCH)-$(MODE)/
MKEXT     ?= mk

compilerof = $(if $(filter-out %.c.o %.m.o,$(filter %.o,$1)),CXX,CC)
canonical  = $(patsubst $(CURDIR)/%,%,$(abspath $1))
outdirof   = $(BUILD)$(if $(suffix $1),lib,bin)
flagsof    = $1.$(if $(filter-out %.c.o %.m.o,$1),cxx,c)flags
print      = $(if $V,$(strip $2),$(if $Q,@$2,@$(if $1,printf $1;) $2))

define add.mk
    sources   := # Project source files, supports wildcards, required
    includes  := # Directories to use as include path for the compiler, supports wildcards
    INCLUDES  :=
    depends   := # Subprojects this project depends on (i.e. the name of their .mk file without the .mk extension)
    cflags    := # C compiler flags
    CFLAGS    :=
    cxxflags  := # C++ compiler flags
    CXXFLAGS  :=
    ldflags   := # Linker flags (don't worry about linking any library subprojects, that happens automatically)
    LDFLAGS   :=

    include $1
    $$(if $$(strip $$(sources)),,$$(error $1: No source files were provided))

    t             := $$(call outdirof,$$(basename $1))/$$(notdir $$(basename $1))
    $$t.sources   := $$(strip $$(wildcard $$(call canonical,$$(sources:%=$$(dir $1)%))))
    $$t.objects   := $$(strip $$($$t.sources:%=$$(BUILD)obj/%.o))
    $$t.includes  := $$(strip $$(addprefix -I,$$(wildcard $$(call canonical,$$(includes:%=$$(dir $1)%)))))
    $$t.INCLUDES  := $$(strip $$(addprefix -I,$$(wildcard $$(call canonical,$$(INCLUDES:%=$$(dir $1)%)))))
    $$t.depends   := $$(strip $$(foreach d,$$(depends),$$(call outdirof,$$d)/$$d) $$($$t.objects) $1)
    $$t.DEPENDS   := $$(strip $$(foreach d,$$(DEPENDS),$$(call outdirof,$$d)/$$d))
    $$t.cflags    := $$(strip $$(cflags) $$($$t.includes))
    $$t.CFLAGS    := $$(strip $$(CFLAGS) $$($$t.INCLUDES))
    $$t.cxxflags  := $$(strip $$(cxxflags) $$($$t.includes))
    $$t.CXXFLAGS  := $$(strip $$(CXXFLAGS) $$($$t.INCLUDES))
    $$t.ldflags   := $$(strip -L$(BUILD)lib $$(patsubst lib%.a,-l%,$$(filter %.a,$$(notdir $$($$t.depends)))) $$(ldflags))
    $$t.LDFLAGS   := $$(strip $$(LDFLAGS))

    $$(foreach o,$$($$t.objects),\
        $$(eval $$o.cflags   := $$($$t.cflags)   $$($$t.CFLAGS))\
        $$(eval $$o.cxxflags := $$($$t.cxxflags) $$($$t.CXXFLAGS)))

    # Add an alias command so you can specify the name of the project as a make argument
    $$(notdir $$(basename $1)): $$t

    targets       += $$t
    files         += $$t $$($$t.objects)
endef

define add.o
    $1.depends := $(1:$(BUILD)obj/%.o=%)
    $1.message := "\033[0;32m%-3s $$($1.depends)\033[0m\n" "$$(call compilerof,$1)"
    $1.command := $$($$(call compilerof,$1)) $$($$(call flagsof,$1)) -MMD -MP -c -o $1 $$($1.depends)
endef

define add.a
    $$(foreach d,$$($1.depends),\
        $$(foreach o,$$($1.objects),\
            $$(eval $$o.cflags += $$($$d.CFLAGS))))

    $1.message := "\033[1;32mAR  $$(notdir $1)\033[0m\n"
    $1.command := $(AR) -rcs $1 $$($1.objects)

    ifneq ($$(strip $$(filter %.a,$$($1.depends))),)
        $1.command += && mkdir -p $1.tmp\
                      && cd $1.tmp\
                      $$(foreach a,$$(filter %.a,$$($1.depends)),\
                        && $(AR) -xo $(CURDIR)/$$a)\
                      && cd - > /dev/null\
                      && $(AR) -rcs $1 $1.tmp/*.o\
                      && rm -r $1.tmp
    endif
endef

define add
    $$(foreach d,$$($1.depends),\
        $$(eval $1.ldflags += $$($$d.LDFLAGS))\
        $$(foreach o,$$($1.objects),\
            $$(eval $$o.cflags += $$($$d.CFLAGS))))

    $1.message := "\033[1;32mLD  $$(notdir $1)\033[0m\n"
    $1.command := $$($$(call compilerof,$$($1.depends))) -o $1 $$($1.objects) $$($1.ldflags) $$($1.LDFLAGS)
endef

ifneq ($(MAKECMDGOALS),clean)
    modules := $(shell find . -name '*.$(MKEXT)' | cut -c3-) # cut -c3- removes the leading ./
    targets :=
    files   :=
endif

all: $$(targets)
.PHONY: all

clean:
> $(call print,"\033[1;33mRM  $(BUILD)\033[0m\n",rm -rf $(BUILD))
.PHONY: clean

run-%: $(BUILD)bin/$$*
> $(call print,"\033[1;36mRUN $^\033[0m\n", ./$^)
.PHONY: run-%

print-%:
> @printf "$* = $($*)\n"
.PHONY: print-%

ifneq ($(MAKECMDGOALS),clean)
    $(foreach m,$(modules),$(eval $(call add.mk,$m)))
    $(foreach f,$(files),$(eval $(call add$(suffix $f),$f)))
endif

$(files): $$($$@.depends)
> $(call print,,mkdir -p $(@D))
> $(call print,$($@.message),$($@.command))

ifneq ($(MAKECMDGOALS),clean)
    -include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
endif
