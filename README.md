# PTP.STM32
PTPv2 grandmaster based on the ARM CMSIS standard interface

TODO:

* get basic TCP/IP example up and running: basic DHCP Autodiscovery vs. Static IP

* Ethernetif Driver:
  - Sort Ethernet Interrupts
  - Get Recieve Demo/ICMP working
  - Add Static IP Override
  - Neaten Driver Code
  - Add IGMP Support
  - Add Link Polling/Detection

* Start Adding PTP functionality to the Ethernet Driver

* get PTP daemon running on STM32F7. Port F4 code

## Reference Software Tools:

* ANEMAN https://www.aneman.net/ : This is similar to Dante Controller, but works
  with a plugin format that means anyone can create a device profile/protocol that 
  can interoperate with an AES67 network.

* Merging VAD https://www.merging.com/products/aes67_vad_standard : OSX only, but
  allows for up to 64 channels of IO with the free version. Plays nicely with ANEMAN.

* Ravenna RVSC https://www.ravenna-network.com/resources/ : a windows-compatible 
  AES67 virtual soundcard which can be used for testing up to 16 channels of IO.

* Open-Source AES67 Daemon https://github.com/bondagit/aes67-linux-daemon : recently
  released, looks good, might free any long-term RAVENNA dependencies. Not really a
  production release yet.

## Project Structure:

* The project is designed to work using the CMSIS interface: this means that
  the core code should run on any Cortex-M microcontroller with appropriate peripherals

* Note that while the CMSIS standard provides a selection of function declarations
  to interact with the MCU hardware, the implementation is up to you/the manufacturer.
  The folder labelled ```dev/``` contains implementations of the CMSIS functions and
  any other low-level setup code.

* To port this project to another ARM MCU, make a device folder following the structure
  illustrated in the ```dev/stm32f767xx/``` folder. Note that the device folder is the ONLY folder
  that can contain device-specific/non-CMSIS-Compliant code. The src code should
  work with all devices without modification.

## Building this Project:

* To build the firmware for all devices, simply run ```make```. ELFs and binaries will
  be generated in the appropriate ```bin/``` folder. Similarly, ```make clean``` will clean
  all devices.

* ```make flash``` will build the code for all devices, then run each device's
  ```prog/'device'/flash.sh``` script. These scripts do not initially do anything,
  and should be set up by the user to suit their programming setup. Some example targets
  are provided at the bottom of the script. If the project is built with
  ```make flash FLAGS=-g```, then the script can be configured to launch a 'debug'
  version of the flash program (e.g. launch GDB, etc...)

* To build/clean/flash only a single device, suffix the make command with ```_'device'```,
  e.g. ```make stm32f767xx```, ```make clean_stm32f767xxx```, ```make flash_stm32f767xxx```, etc...
