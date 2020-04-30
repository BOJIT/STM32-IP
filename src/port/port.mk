## Port makefile flags/configuration (i.e. device-specific settings):

# Port name is used for reference/folder naming only (it can be any string)
PORT_NAME := Nucleo_STM32F767xx

# Port-specific compiler flags
PORT_CFLAGS := -mcpu=cortex-m7 -mthumb # Architecture flags
PORT_CFLAGS += -mfloat-abi=hard -mfpu=fpv5-sp-d16 # Architecture flags
PORT_CFLAGS += -DSTM32F7 # Libopencm3 definition
PORT_CFLAGS += -Isrc/port/inc # Global include directory

# Port-specific linker flags
PORT_LFLAGS := -Tsrc/port/stm32f767xx.ld # Linker script

# Port-specific libopencm3 library
PORT_LIBOPENCM3 := libopencm3_stm32f4.a

# Port-specific FreeRTOS architecture
PORT_FREERTOS := GCC/ARM_CM7/r0p1
