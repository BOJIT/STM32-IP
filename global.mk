## Global Makefile flags (same for all ARM Cortex-M MCUs):

# Set project name
PROJECT := PTP_STM32

# Build toolchains
CC      := arm-none-eabi-gcc
AR      := arm-none-eabi-ar
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

# Warning flags
_WARN_FLAGS := -Wall -pedantic -Wsign-compare #-Wcast-align

# Optimisation flags
_OPT_FLAGS := -fno-common -ffunction-sections -fdata-sections #-Os
_OPT_FLAGS += -fsingle-precision-constant -fomit-frame-pointer

# Global compiler flags
CFLAGS := $(_WARN_FLAGS) $(_OPT_FLAGS) $(FLAGS) # Used to get cmd from terminal
CFLAGS += -Iinc # Global include directory

# Global linker flags
LFLAGS := -Wl,--gc-sections
LFLAGS += --specs=nosys.specs --specs=nano.specs
LFLAGS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
LFLAGS += --static -nostartfiles

# Pretty Terminal Formatting
FORMAT_PURPLE = \033[1m\033[95m
FORMAT_WHITE = \033[1m\033[36m
FORMAT_RED = \033[1m\033[91m
FORMAT_OFF = \033[0m
