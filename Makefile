## Main project Makefile:

# Variable Naming Scheme:

# - Variables that are of local scope should start with underscore (_EXAMPLE)
# - Variables that are exported should begin with a lowercase 'e' (eEXAMPLE)
# - Variables that are port-specific should be prefixed 'PORT_' (PORT_EXAMPLE)

################################## SOURCES #####################################

# Project Source Files:
#SRC := $(wildcard src/*.c)
#INC := $(sort $(dir $(shell find inc -name "*.h")))

_LIB := $(wildcard lib/*.mk)

############################# MAKEFILE GLOBALS #################################

# Import global and port-specific flags/variables
include global.mk
include src/port/port.mk

# Pretty Terminal Formatting
_FORMAT_PURPLE = \033[1m\033[95m
_FORMAT_WHITE = \033[1m\033[36m
_FORMAT_RED = \033[1m\033[91m
_FORMAT_OFF = \033[0m

################################## TARGETS #####################################

.PHONY: all clean flash $(_LIB)

all: $(_LIB)

clean:
    # Delete device binary and build directories
	@echo "$(_FORMAT_PURPLE)Cleaning Project:$(_FORMAT_OFF)"
	rm -rf bin/
	rm -rf build/

flash:
	# Run shell script that contains custom flashing scripts
	@echo "$(_FORMAT_RED)DOING NOTHING! $@$(_FORMAT_OFF)"

# $(DEVICES): % : dep_%
# 	@echo "$(_FORMAT_PURPLE)Building and Linking $@$(_FORMAT_OFF)"
# 	mkdir -p bin/$@
# 	# Get dependencies
# 	$(eval CDEPS = $(shell find build/$@ -name "*.cdep"))
# 	$(eval CDEPFLAGS = )
# 	$(foreach DEP, $(CDEPS), $(eval CDEPFLAGS += $(shell cat $(DEP))))
# 	$(eval LDEPS = $(shell find build/$@ -name "*.ldep"))
# 	$(eval LDEPFLAGS = )
# 	$(foreach DEP, $(LDEPS), $(eval LDEPFLAGS += $(shell cat $(DEP))))
# 	# GCC build and link goes here
# 	$(CC) $(CFLAGS) $(SRC) $(CDEPFLAGS) $(addprefix -I, $(INC)) \
# 	-Wl,-Map=bin/$@/$@.map -o bin/$@/$@.elf $(LFLAGS) $(LDEPFLAGS)
# 	# Copy from ELF to binary format
# 	$(OBJCOPY) -O binary bin/$@/$@.elf bin/$@/$@.bin

$(_LIB):
	@echo "$(_FORMAT_PURPLE)Building $@:$(_FORMAT_OFF)"
	$(MAKE) -f$@
