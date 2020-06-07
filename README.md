# STM32-IP
## Basic framework for using lwIP and FreeRTOS with libopencm3:

This framework is completely independent of the STM32 HAL, and is designed with a zero-copy Ethernet driver that supports PTP.
This framework is also designed to be cross-platform, and easily portable to other ARM Cortex-M microcontrollers.

TODO:

* get basic TCP/IP example up and running: basic DHCP Autodiscovery vs. Static IP

* Ethernetif Driver:
  - Sort Ethernet Interrupts
  - Get Recieve Demo/ICMP working
  - Add Static IP Override
  - Neaten Driver Code
  - Add IGMP Support
  - Add Link Polling/Detection
  - Finish Doxygen documentation

* Start Adding PTP functionality to the Ethernet Driver

* get PTP daemon running on STM32F7. Port F4 code

## Reference Software Tools:

* ANEMAN https://www.aneman.net/ : This is similar to Dante Controller, but works with a plugin format that means anyone can create a device profile/protocol that can interoperate with an AES67 network.

* Merging VAD https://www.merging.com/products/aes67_vad_standard : OSX only, but allows for up to 64 channels of IO with the free version. Plays nicely with ANEMAN.

* Ravenna RVSC https://www.ravenna-network.com/resources/ : a windows-compatible AES67 virtual soundcard which can be used for testing up to 16 channels of IO.

* Open-Source AES67 Daemon https://github.com/bondagit/aes67-linux-daemon : recently released, looks good, might free any long-term RAVENNA dependencies. Not really a production release yet.

## Project Structure:

* The project is designed to work using the CMSIS interface: this means that the core code should run on any Cortex-M microcontroller with appropriate peripherals

* Note that while the CMSIS standard provides a selection of function declarations to interact with the MCU hardware, the implementation is up to you/the manufacturer. The folder labelled ```port/``` contains implementations of any port-specific functions. These have to be re-written if porting to another architecture.

## Building this Project:

* To build the firmware for all devices, simply run ```make```. ELFs and binaries will be generated in the appropriate ```bin/``` folder. Similarly, ```make clean``` will clean all devices.
