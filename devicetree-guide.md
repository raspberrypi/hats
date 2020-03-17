# HAT Devicetree Blob guide

### **This section is under active development and portions of the guide rely on software features not yet implemented.**
---
Within the HAT specification is the requirement to include a data atom that contains a devicetree overlay binary (`.dtbo` or `-overlay.dtb`).

First, read and understand what a device tree is and how it is used by system software (i.e. Linux).

[Device Tree Usage](http://elinux.org/Device_Tree_Usage)

[Device Tree For Dummies](http://events.linuxfoundation.org/sites/events/files/slides/petazzoni-device-tree-dummies.pdf)

An extension to this mechanism of hardware description is the introduction of an overlay: a partially-complete devicetree fragment that "joins up" to a higher-level device tree. The basic idea is that this overlay will be read on boot (by the VC bootloader), parsed and merged into the system-level device tree passed through to the ARM running Linux.

The purpose of this overlay is that it allows true automatic configuration of all devices attached to the HAT, as long as there are Linux device drivers available for the hardware in question. 

The devicetree overlay will be of particular use to add-on board designers making use of the I2C-1, SPI and I2S buses that require chip selects / addresses to be specified in order to be probed and set up correctly by the respective Linux device driver, but it will also allow simple devices such as LEDs and buttons to be recognised by Linux and connected up to e.g. the kernel input events subsystem.

The process of writing an overlay is outside the scope of this document. Readers are instead referred to this Raspberry Pi document:

[Device Trees, Overlays and Parameters](http://www.raspberrypi.org/documentation/configuration/device-tree.md)

### HAT identification

To avoid the need for modules and applications to access the EEPROM, the VC bootloader reads the vendor, product and custom data and populates the `/hat` node of the device tree. This can be read by kernel modules using the standard DT API (`of_*`), or via `/proc/device-tree/hat` from user-space. Here is an example of the sort of information you will find there:
```
/proc/device-tree/hat/vendor:
ACME Corp.

/proc/device-tree/hat/product:
Dunce HAT

/proc/device-tree/hat/product_id:
0x0001

/proc/device-tree/hat/product_ver:
0x0001

/proc/device-tree/hat/uuid:
c617cf6a-7de5-4948-0000-000001b86eb0

/proc/device-tree/hat/custom_0:
The boy stood on the burning deck
His lips were all a-quiver.

/proc/device-tree/hat/custom_1:
Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
```
The custom blobs are copied verbatim, and may contain arbitrary binary data.

It is permissible for the overlay to supplement this data with additional information, but the standard properties always take precedence, overwriting any user-supplied values. Conversely, if there is no HAT present then the `/hat` node is deleted just before boot.

### Pinctrl nodes vs GPIO map

Within the devicetree fragment is scope for adding pinctrl nodes that alter the setup of GPIO pins. The timing or sequencing used by pinctrl for GPIO setup may be non-trivial: for example a GPIO could be nominated as a reset pin for an external MCU that may require a reset pulse of a certain length, or a GPIO could be nominated to enable an external power chain that requires a certain period of stabilisation before attempting to talk to any attached devices. The Linux Pinctrl subsystem is designed to cater for complex requirements such as these.

The GPIO pin data map in the EEPROM is **still required** even if pinctrl configuration nodes are specified in the device tree blob. The GPIO map is parsed by the Videocore bootloader prior to ARM boot and **not** Linux.
