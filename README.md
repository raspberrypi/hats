# B+ HAT DESIGN GUIDE

**NOTE THIS INFORMATION IS CURRENTLY STILL CHANGING**

## Introduction

The Raspberry Pi B+ has been designed specifically with add-on boards - we're calling them Raspberry Pi HATs (Hardware Attached on Top) - in mind. This guide contains both recommendations and also **requirements** that must be followed when designing a HAT for the B+.

**If you are thinking about or are designing an add-on board for the Raspberry Pi B+ please read and follow this design guide carefully and make sure the requirements are met. There aren't too many hard requirements, and they shouldn't be too difficult to design for or to follow.**

While we cannot force anyone to follow this guide, any boards which break the rules (and therefore may cause incompatibility or issues for end users) will not be looked on very favourably.

So why are we bothering with all this? Basically we want to ensure consistency and compatibility with future add-on boards, and to allow a much better end-user experience, especially for the less technically aware users.

Finally if you have any questions please head over to the [forums](http://www.raspberrypi.org/forums/viewforum.php?f=100) to ask them.

## Using GPIO Pins

### Power-on State

In the new B+ firmware after power-on the bank 0 GPIOs on GPIO header J8 (except ID_SD and ID_SC which are GPIO0 and 1 respectiveley) will be inputs with either a pull up or pull down. The default pull state can be found in the [BCM2835 peripherals specificaion](http://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf) section 6.2 table 6-31 (see the "Pull" column).

### Notes and Recommendations

GPIO pins ID_SC and ID_SD (GPIO0 and GPIO1) are reserved for use solely for board identification. **An I2C EEPROM plus pull-up resistors should be the only connections to these pins.**

Raspberry Pi models A and B use some bank 0 GPIOs for board control functions and UART output:

    GPIO6 -> LAN_RUN
    GPIO14 -> UART_TX
    GPIO16 -> STATUS_LED

**If a user boots a B+ with legacy firmware these pins may get driven so it is recommended to avoid driving these from a HAT, or use a current limiting resistor if that is not possible. Note also that relying on the pull state of these pins during boot is not advisable.** 

It is also strongly recommended to ship a clear warning notice with your HAT that updated firmware must be used, and clear instructions for how to update the firmware on the Pi before plugging in the HAT.

## ID EEPROM

Within the set of pins available on the J8 GPIO header, ID_SC and ID_SD (GPIO0/SCL and GPIO1/SDA) **are reserved solely for attaching an I2C 'ID' EEPROM, do not connect anything else to these pins**.

The ID EEPROM is interrogated at boot time and provides the Pi with the required GPIO setup (pin settings and functions) for the HAT as well as a binary Linux device tree fragment which also specifies which hardware is used and therefore which drivers need loading. EEPROM information is also available to userland Linux software for identifying attached boards. Note that the device tree fragment is optional but strongly recommended. (NB docs for how to create this are on their way...)

Pull-ups must be provided on the top board for ID_SC and ID_SD  (SCL and SDA respectively) to 3V3. The recommended pull-up value is 3.9K.

A 24Cxx type I2C EEPROM must be used. The device must be powered from 3V3 (avoid the 5V only variants) and only needs to support 100kHz I2C mode. Devices that perform I2C clock stretching are not supported.

A recommended part is OnSemi CAT24Cxx, for example CAT24C32 for a 32kbit device. The minimum EEPROM size required is variable and depends on the size of the vendor data strings in the EEPROM and whether a device tree data blob is included (and its size) and whether any other vendor specific data is included.

It is recommended that EEPROM WP (write protect) pin be connected to a test point on the board and pulled up to 3V3 with a 1K resistor. The idea is that at board test/probe the EEPROM can be written (WP pin can be driven LOW), but there is no danger of a user accidentally changing the device contents once the board leaves the factory. Note that the recommended device has an internal pull down hence the stiff (1K) pull up is required. Note that on some devices WP does not write protect the entire array (e.g. some Microchip variants) – avoid using these.
It may be desirable for a HAT to have the ability for its EEPROM to be reflashed by an end user, in this case it is recommended to also include a user settable jumper (or dip switch) to short WP to GND and make the EEPROM writable once more. At least this way a user has to perform a specific action to make the EEPROM writeable again before being able to re-flash it and a suitable warning process can be put in place to make sure the correct image is used.

Address pins where present on a device should be set to zero. (NB reduced pin count variants of the recommended device – e.g. SOT23-5 package - usually have A[2:0] set to 0 anyway).

Details of the EEPROM format can be found in the [EEPROM format specification](eeprom-format.md). [Software tools](./eepromutils) are available for creation of valid EEPROM images, to flash an image or read and dump and image to/from an attached HAT EEPROM.

[The following schematic fragment](eeprom-circuit.png) is an example of connecting an EEPROM  including a jumper and probe point to disable write protect.

## Mechanical Specification

The [following drawing](hat-board-mechanical.pdf) gives mechanical detials of HATs intended to fit over the 4 mounting holes (56x65mm size boards).

## Back Powering the Pi via the J8 GPIO Header

It is possible to power the Pi by supplying 5V thourhg the GPIO header (J8) pins 2,4 and GND. The acceptable input voltage range is 5V ±5%.

On the Pi, the 5V GPIO header pins connect to the 5V net after the micro-USB input, polyfuse and input 'ideal' safety diode (made up of the PFET and matched PNP transistors). The 'safety' diode stops any appreciable current flowing back out of the 5V micro USB should the 5V net on the board be at a higher voltage than the 5V micro USB input.

If the Pi is going to be back-powered via the 5V GPIO header pins it is **strongly recommended** to implement a duplicate power safety diode before the HAT 5V net (which then feeds power back through the 5V GPIO pins).

Implementing the back-powering this way makes it safe to provide power to the Pi from the HAT board but also have the Pi 5V power supply attached. (Though it is still not recommended to attach both).

**Under no circumstances should a power source be connected to the J8 3.3V pins.**
