# Note that ROOT_DIR should be exported from the top-level makefile

############################### CONFIGURATION ##################################

DEVICE-NAME := stm32f407xx

DEVICE-FLAGS := -mcpu=cortex-m4 -mthumb
DEVICE-FLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

# Select relevant libopencm3 library
DEVICE-FRAMEWORK := libopencm3_stm32f4.a

# Set FreeRTOS build settings
DEVICE-RTOS := GCC/ARM_CM4F
DEVICE-RTOS-CONFIG := -I$(ROOT_DIR)/dev/$(DEVICE-NAME)

# Build directory of device-specific objects
BUILD_DIR := $(ROOT_DIR)/build/$(DEVICE-NAME)/dev

OBJECTS := $(BUILD_DIR)/device_functions.o
INC_DEF := -I$(ROOT_DIR)/lib/libopencm3/include  -DSTM32F4

CDEPS  := $(OBJECTS) -I$(ROOT_DIR)/dev/$(DEVICE-NAME) $(DEVICE-FLAGS)
LDEPS := -T$(ROOT_DIR)/dev/$(DEVICE-NAME)/$(DEVICE-NAME).ld

############################# MAKEFILE GLOBALS #################################

# Include global headers/compiler flags
include $(ROOT_DIR)/global.mk

# Export device-specific headers for use by libraries
export DEVICE-NAME
export DEVICE-FLAGS
export DEVICE-FRAMEWORK
export DEVICE-RTOS
export DEVICE-RTOS-CONFIG

LIBS := $(patsubst $(ROOT_DIR)/lib/%.mk,%,$(wildcard $(ROOT_DIR)/lib/*.mk))

CFLAGS += -c # Don't link files yet	

################################## TARGETS #####################################

default: $(OBJECTS)
	# Build all libraries for each device with appropriate flags
	$(foreach LIB,$(LIBS),$(MAKE) -C$(ROOT_DIR)/lib -f$(patsubst %,%.mk,$(LIB));)
	touch $(BUILD_DIR)/dev.cdep
	echo "$(CDEPS)" > $(BUILD_DIR)/dev.cdep
	touch $(BUILD_DIR)/dev.ldep
	echo "$(LDEPS)" > $(BUILD_DIR)/dev.ldep


$(BUILD_DIR)/%.o: $(DEVICE-NAME)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC_DEF) $(DEVICE-FLAGS) $< -o $@

$(BUILD_DIR)/%.o: $(DEVICE-NAME)/%.S
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC_DEF) $(DEVICE-FLAGS) $< -o $@
