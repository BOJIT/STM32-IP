# lwIP Makefile

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
_LIB_NAME = lwip

_BUILD_DIR = build/lib/$(_LIB_NAME)
_LIB_DIR = lib/$(_LIB_NAME)

_LIB_LDEPS := -L$(_BUILD_DIR) -l$(_LIB_NAME)

################################## TARGETS #####################################

default:
	@echo "$(_FORMAT_RED)Building lwip:$(_FORMAT_OFF)"
