# FreeRTOS-Kernel Makefile

############################# MAKEFILE GLOBALS #################################

# Import global and port-specific flags/variables
include global.mk
include src/port/port.mk

################################## SOURCES #####################################

# Set library name (should be equivalent to the folder/submodule name)
_LIB_NAME = FreeRTOS-Kernel

_BUILD_DIR = build/lib/$(_LIB_NAME)
_LIB_DIR = lib/$(_LIB_NAME)

_SRC := croutine.c event_groups.c list.c queue.c stream_buffer.c tasks.c timers.c
_SRC += portable/MemMang/heap_4.c portable/$(PORT_FREERTOS)/port.c

_OBJ := $(patsubst %.c,$(_BUILD_DIR)/obj/%.o,$(_SRC))

_LIB_CDEPS := -I$(_LIB_DIR)/include -I$(_LIB_DIR)/portable/$(PORT_FREERTOS)

_LIB_LDEPS := -L$(_BUILD_DIR) -l$(_LIB_NAME)

################################## TARGETS #####################################

# Archive into static library
$(_BUILD_DIR)/lib$(_LIB_NAME).a: $(_OBJ)
	mkdir -p $(@D)
	$(AR) rcs $@ $^
    # Echo compiler dependencies into dependencies file
	touch build/$(PROJECT).cdep
	echo "$(_LIB_CDEPS)" >> build/$(PROJECT).cdep
    # Echo linker dependencies into dependencies file
	touch build/$(PROJECT).ldep
	echo "$(_LIB_LDEPS)" >> build/$(PROJECT).ldep

# Build all library objects
$(_OBJ): $(_BUILD_DIR)/obj/%.o : $(_LIB_DIR)/%.c
    # Don't link objects yet
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(PORT_CFLAGS) $(_LIB_CDEPS) $^ -o $@
	

