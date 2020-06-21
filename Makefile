## Main project Makefile:

# Variable Naming Scheme:

# - Variables that are of local scope should start with underscore (_EXAMPLE)
# - Variables that are exported should begin with a lowercase 'e' (eEXAMPLE)
# - Variables that are port-specific should be prefixed 'PORT_' (PORT_EXAMPLE)

############################# MAKEFILE GLOBALS #################################

# Import global and port-specific flags/variables
include global.mk
include src/port/port.mk

################################## SOURCES #####################################

_BUILD_DIR = build

# Project Source Files:
_SRC := $(shell find src -name "*.c")
_LIB := $(wildcard lib/*.mk)
_OBJ := $(patsubst %.c,$(_BUILD_DIR)/%.o,$(_SRC))

################################## TARGETS #####################################

.PHONY: all clean flash $(_LIB) dep

# Order of operations:
# -> build libs -> read dependencies -> build src -> link everything -> bin

# Link all object files and libraries
all: dep $(_OBJ)
	@echo "$(FORMAT_PURPLE)Linking Application Code:$(FORMAT_OFF)"
	mkdir -p bin
	# CFlags are included purely for setting implicit linker flags
	$(CC) $(CFLAGS) $(PORT_CFLAGS) $(_OBJ) -o bin/$(PROJECT).elf \
	$(LFLAGS) $(PORT_LFLAGS)
	$(OBJCOPY) -O binary bin/$(PROJECT).elf bin/$(PROJECT).bin

# Delete device binary and build directories
clean:
	@echo "$(FORMAT_PURPLE)Cleaning Project:$(FORMAT_OFF)"
	rm -rf bin
	rm -rf build

# Run shell script that contains custom flashing scripts
flash: all
	./src/port/flash.sh bin/$(PROJECT).bin $(FLAGS)

# Build all libs with a sub-make
$(_LIB):
	@echo "$(FORMAT_PURPLE)Building $@:$(FORMAT_OFF)"
	$(MAKE) -f$@

# Get lib dependencies and add them to the compiler/linker flags
dep: $(_LIB)
	@echo "$(FORMAT_PURPLE)Compiling Application Code:$(FORMAT_OFF)"
	$(eval CFLAGS += $(shell cat build/$(PROJECT).cdep))
	$(eval LFLAGS += $(shell cat build/$(PROJECT).ldep))


# Build all source files into objects
$(_OBJ): $(_BUILD_DIR)/%.o : %.c
    # Don't link objects yet
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(PORT_CFLAGS) $^ -o $@
