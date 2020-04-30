# libopencm3 Makefile: simply passes everything down to the lower makefile

############################# MAKEFILE GLOBALS #################################

# Import global and port-specific flags/variables
include global.mk
include src/port/port.mk

# Pretty Terminal Formatting
_FORMAT_PURPLE = \033[1m\033[95m
_FORMAT_WHITE = \033[1m\033[36m
_FORMAT_RED = \033[1m\033[91m
_FORMAT_OFF = \033[0m

################################## SOURCES #####################################

# Set library name (should be equivalent to the folder/submodule name)
_LIB_NAME = libopencm3

_BUILD_DIR = build/lib/$(_LIB_NAME)
_LIB_DIR = lib/$(_LIB_NAME)

_LIB_CDEPS := -I$(_LIB_DIR)/include

_LIB_LDEPS := -L$(_BUILD_DIR) -l$(patsubst lib%.a,%,$(PORT_LIBOPENCM3))

################################## TARGETS #####################################

# Target will only rebuild if the build directory is cleaned
$(_BUILD_DIR)/$(PORT_LIBOPENCM3):
	mkdir -p $(@D)
    # Make libopencm3 (builds for all supported platforms)
	$(MAKE) -C $(_LIB_DIR)
    # Copy the desired static library to the build folder
	cp $(_LIB_DIR)/lib/$(PORT_LIBOPENCM3) $@
    # Echo compiler dependencies into dependencies file
	touch build/$(PROJECT).cdep
	echo "$(_LIB_CDEPS)" >> build/$(PROJECT).cdep
    # Echo linker dependencies into dependencies file
	touch build/$(PROJECT).ldep
	echo "$(_LIB_LDEPS)" >> build/$(PROJECT).ldep
