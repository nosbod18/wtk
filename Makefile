MAKEFLAGS += -r
SHELL := /bin/bash
.SUFFIXES:
.SECONDEXPANSION:
.RECIPEPREFIX := >

OS        ?= $(shell uname -s)
ARCH      ?= $(shell uname -m)
MODE      ?= debug
BUILD     ?= .build/$(OS)-$(ARCH)-$(MODE)/
MKEXT     ?= mk

ifndef progress
    total   != $(MAKE) $(MAKECMDGOALS) -nrR progress="__here" | grep -c "__here"
    n       := x
    count    = $(words $n)$(eval n := x $n)
    progress = printf "[%3d%%] " "$$(($(count) * 100 / $(total)))"
endif

compilerOf = $(if $(filter-out %.c.o %.m.o,$(filter %.o,$1)),CXX,CC)
canonical  = $(patsubst $(CURDIR)/%,%,$(abspath $1))
outdirOf   = $(BUILD)$(if $(suffix $1),lib,bin)
flagsOf    = $1.$(if $(filter-out %.c.o %.m.o,$1),cxx,c)flags
print      = $(if $V,$(strip $2),$(if $Q,@$2,@$(if $1,$(progress); printf $1;) $2))

define add.mk
    sources   := # Project source files, supports wildcards, required
    includes  := # Directories to use as include path for the compiler, supports wildcards
    INCLUDES  :=
    depends   := # Subprojects this project depends on (i.e. the name of their .mk file without the .mk extension)
    cflags    := # count compiler flags
    CFLAGS    :=
    cxxflags  := # CXX compiler flags
    CXXFLAGS  :=
    ldflags   := # Linker flags (don't worry about linking any library subprojects, that happens automatically)
    LDFLAGS   :=

    include $1
    $$(if $$(strip $$(sources)),,$$(error $1: No source files were provided))

    t             := $$(call outdirOf,$$(basename $1))/$$(notdir $$(basename $1))
    $$t.sources   := $$(strip $$(wildcard $$(call canonical,$$(sources:%=$$(dir $1)%))))
    $$t.objects   := $$(strip $$($$t.sources:%=$$(BUILD)obj/%.o))
    $$t.includes  := $$(strip $$(addprefix -I,$$(wildcard $$(call canonical,$$(includes:%=$$(dir $1)%)))))
    $$t.INCLUDES  := $$(strip $$(addprefix -I,$$(wildcard $$(call canonical,$$(INCLUDES:%=$$(dir $1)%)))))
    $$t.depends   := $$(strip $$(foreach d,$$(depends),$$(call outdirOf,$$d)/$$d) $$($$t.objects) $1)
    $$t.DEPENDS   := $$(strip $$(foreach d,$$(DEPENDS),$$(call outdirOf,$$d)/$$d))
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
    $1.message := "\033[0;32m%-3s $$($1.depends)\033[0m\n" "$$(call compilerOf,$1)"
    $1.command := $$($$(call compilerOf,$1)) $$($$(call flagsOf,$1)) -MMD -MP -c -o $1 $$($1.depends)
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
    $1.command := $$($$(call compilerOf,$$($1.depends))) -o $1 $$($1.objects) $$($1.ldflags) $$($1.LDFLAGS)
endef

ifneq ($(MAKECOMMANDGOALS),clean)
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

ifneq ($(MAKECOMMANDGOALS),clean)
    $(foreach m,$(modules),$(eval $(call add.mk,$m)))
    $(foreach f,$(files),$(eval $(call add$(suffix $f),$f)))
endif

$(files): $$($$@.depends)
> $(call print,,mkdir -p $(@D))
> $(call print,$($@.message),$($@.command))

ifneq ($(MAKECOMMANDGOALS),clean)
    -include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
endif
