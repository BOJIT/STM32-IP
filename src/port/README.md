# Required Port-Specific Components:

- ```port_functions.c``` - contains all port-specific functions that must be
  provided when porting to a new device (these functions are declared in
  ```port_functions.h```)

- ```inc/port_config.h``` - contains port-specific definitions (clock speed,
  RTOS commands, etc...)

- ```***.ld***``` - linker script for the target processor. The name of the file
  is defined in ```port.mk```.

- ```port.mk``` - included makefile which defines port-specific flags,
  frameworks, linker scripts and RTOS build settings.
