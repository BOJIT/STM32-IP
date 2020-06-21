# lwIP Makefile (tailored for building with GCC and FreeRTOS)

############################# MAKEFILE GLOBALS #################################

# Import global and port-specific flags/variables
include global.mk
include src/port/port.mk

################################## SOURCES #####################################

################################## TARGETS #####################################

default:

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
$(_OBJ): $(_BUILD_DIR)/obj/%.o : %.c
	echo $(_SRC)
    # Don't link objects yet
	mkdir -p $(@D)
	$(CC) -c $(CFLAGS) $(PORT_CFLAGS) $(_LIB_CDEPS) $^ -o $@
