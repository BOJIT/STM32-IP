Porting  {#page}
================

# Porting Guide:

THIS DOCUMENT NEEDS UPDATING!

- ```inc/port_config.h``` - contains port-specific definitions (clock speed,
  RTOS commands, etc...)

- ```***.ld``` - linker script for the target processor. The name of the file
  is defined in ```port.mk```.

- ```port.mk``` - included makefile which defines port-specific flags,
  frameworks, linker scripts and RTOS build settings.

- ```flash.sh``` - this is the script that is called when 'make flash' is run.
  It is optional, and by default does nothing, but flashing commands such as 
  ```st-flash```. The script is passed any extra makefile flags as arguments, 
  and can be configured to run different instructions if ```-g``` is passed.
