# HAT Device Tree Blob guide

Within the HAT specification is the requirement to include a data atom that contains either the name or the content of a Device Tree overlay.

First, read and understand what a Device Tree is and how it is used by system software (i.e. Linux).

[Device Tree Usage](http://elinux.org/Device_Tree_Usage)

[Device Tree For Dummies](http://events.linuxfoundation.org/sites/events/files/slides/petazzoni-device-tree-dummies.pdf)

An extension to this mechanism of hardware description is the introduction of an overlay: a partially-complete DT fragment that "joins up" to a higher-level Device Tree. The basic idea is that this overlay will be read on boot (by the VC bootloader), parsed and merged into the system-level Device Tree passed through to the ARM running Linux.

The purpose of this overlay is that it allows true automatic configuration of all devices attached to the HAT, as long as there are Linux device drivers available for the hardware in question. 

The DT overlay will be of particular use to add-on board designers making use of the I2C-1, SPI and I2S buses that require chip selects / addresses to be specified in order to be probed and set up correctly by the respective Linux device driver, but it will also allow simple devices such as LEDs and buttons to be recognised by Linux and connected up to e.g. the kernel input events subsystem.

The process of writing an overlay is outside the scope of this document. Readers are instead referred to this Raspberry Pi document:

[Device Trees, Overlays and Parameters](http://www.raspberrypi.org/documentation/configuration/device-tree.md)

### Populating the Device Tree Atom

There are two approaches to providing an overlay for a HAT:

#### a) Embed the overlay in the EEPROM

You can build an overlay into the EEPROM image (`*.eep`) with the following command:
```
$ eepmake mysettings.txt myhat.eep myoverlay.dtbo
```
This assumes that your settings file is called `mysettings.txt`, the overlay is called `myoverlay.dtbo`, and you want the output to go to `myhat.eep` - modify the names as appropriate.

At boot time, the firmware will read the overlay directly from the EEPROM and apply it. Building the overlay in guarantees that the user has the it installed. However, all but the simplest hardware is likely to require a custom driver, and having an overlay without the necessary drivers is no better than not having the overlay, so users may still need to update their OS.

#### b) Put the name of the overlay in the EEPROM

Alternatively, you can build just the name of overlay into the EEPROM image like this:
```
$ echo myoverlay > myoverlay_name.txt
$ eepmake mysettings.txt myhat.eep myoverlay_name.txt
```
It doesn't matter about trailing whitespace in the text file - the firmware will ignore it.

When the firmware reads the EEPROM during boot it detects that the atom content doesn't look a `.dtbo` file, and tries to interpret it as the name of an overlay to load in the usual way. This relies on the overlay being installed, but has the advantage that it is easier to update the overlay should it become necessary.

### Adding overlays and drivers to the Raspberry Pi kernel

Some HATs will require custom drivers and/or an overlay. The source for the Raspberry Pi kernels and overlays can be found in the [Raspberry Pi Linux GitHub repo](https://github.com/raspberrypi/linux). Submitting code for inclusion in the Raspberry Pi kernel repo is fairly easy (if you understand git, harder if you don't):

1. Register for a GitHub account (free in most cases).
2. Clone the repo, make changes and commit them.
3. Fork the repo using the GitHub GUI.
4. Push the updates to your fork.
5. Create a Pull Request.

GitHub have [their own documentation for this](https://help.github.com/en/github/collaborating-with-issues-and-pull-requests), but there is a more user-friendly introduction [here](https://opensource.com/article/19/7/create-pull-request-github).

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

The GPIO pin data map in the EEPROM is **still required** even if pinctrl configuration nodes are specified in the DT blob. The GPIO map is parsed by the Videocore bootloader prior to ARM boot and **not** Linux.
