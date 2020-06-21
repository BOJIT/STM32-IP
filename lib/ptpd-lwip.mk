# lwIP Makefile (tailored for building with GCC and FreeRTOS)

############################# MAKEFILE GLOBALS #################################

# Import global and port-specific flags/variables
include global.mk
include src/port/port.mk

################################## SOURCES #####################################

# Set library name (should be equivalent to the folder/submodule name)
_LIB_NAME = ptpd-lwip

_BUILD_DIR = build/lib/$(_LIB_NAME)
_LIB_DIR = lib/$(_LIB_NAME)/src

# Could be optimised? sort when rearranging later
_SRC := arith.c bmc.c protocol.c ptpd.c dep/msg.c
_SRC += dep/net.c dep/servo.c dep/startup.c dep/sys_time.c dep/timer.c

_OBJ := $(patsubst %.c,$(_BUILD_DIR)/obj/%.o,$(_SRC))

_LIB_CDEPS := -I$(_LIB_DIR) -Ilib/lwip/src/include
# temporary - later build will have a public interface and no lwip hard-code will be needed

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
	echo $(_SRC)
    # Don't link objects yet
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(PORT_CFLAGS) $(_LIB_CDEPS) $^ -o $@
