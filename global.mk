CC      = arm-none-eabi-gcc
AR      = arm-none-eabi-ar
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

USER_FLAGS = -Wall -pedantic -Wsign-compare #-Wcast-align
OPT_FLAGS = -fno-common -ffunction-sections -fdata-sections
OPT_FLAGS += -fsingle-precision-constant -fomit-frame-pointer

CFLAGS = $(FLAGS) $(USER_FLAGS) $(OPT_FLAGS) # FLAGS is used to get user flags

LFLAGS = -Wl,--gc-sections
LFLAGS += --specs=nosys.specs
LFLAGS += --specs=nano.specs
LFLAGS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group
LFLAGS += --static -nostartfiles
