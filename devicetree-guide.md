# HAT Devicetree Blob guide

###**This section is under active development and portions of the guide rely on software features not yet implemented.**
---
Within the HAT specification is the requirement to include a data atom that contains a devicetree overlay binary (.dtbo).

First, read and understand what a device tree is and how it is used by system software (i.e. Linux).

[Device Tree Usage](http://devicetree.org/Device_Tree_Usage)

[Device Tree For Dummies](http://events.linuxfoundation.org/sites/events/files/slides/petazzoni-device-tree-dummies.pdf)

An extension to this mechanism of hardware description is the introduction of an overlay: a partially-complete devicetree fragment that "joins up" to a higher-level device tree. The basic idea is that this overlay will be read on boot (by the VC bootloader), parsed and merged into the system-level device tree passed through to the ARM running Linux.

The purpose of this overlay is that it allows true automatic configuration of all devices attached to the HAT, as long as there are Linux device drivers available for the hardware in question. 

The devicetree overlay will be of particular use to add-on board designers making use of the I2C-1, SPI and I2S buses that require chip selects / addresses to be specified in order to be probed and set up correctly by the respective Linux device driver, but it will also allow simple devices such as LEDs and buttons to be recognised by Linux and connected up to e.g. the kernel input events subsystem.

Note: Devices requiring the use of interfaces that need modification of the B+ GPIO header pin states from the default state (Input with default pull) must add a pinctrl pin configuration node that signals to the Linux Pinctrl driver the required configuration. This is **distinct and separate** from Videocore GPIOMAN configuration, as GPIOMAN config is designed for very early setup of all GPIOs prior to booting Linux, which then has notional control over the setup of the Bank 0 GPIO pins.


TODO: Add detail for DTC compiler usage, overlay functionality.