# ADD-ON BOARD / HAT DESIGN GUIDE
---

## GPIO Pins

**NOTE** All references to GPIO numbers within this document are referring to the BCM283x GPIOs (**NOT** pin numbers on the GPIO header).

## GPIO Configuration Sequencing

The default state for all Bank 0 pins will be inputs with either a pull up or pull down. The default pull state can be found in the [BCM2835 peripherals specificaion](http://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2835/BCM2835-ARM-Peripherals.pdf) section 6.2 table 6-31 (see the "Pull" column). It will require positive software configuration to change the state of these pins.

The one exception to this rule is ID_SC and ID_SD. GPIO0 (ID\_SD) and GPIO1 (ID\_SC) will be switched to ALT0 (I2C-0) mode and probed for an EEPROM at boot time. These pins will revert to inputs once the probe sequence has completed.

### Videocore GPIO setup

**As of 11/03/15, this section has been (largely) implemented in firmware. Only the gpio_hysteresis, gpio_slew and back_power are currently unsupported. **

At a very early point in the boot process, either GPIOMAN (for recent firmware) or the Videocore platform code sets the initial state of GPIO pins.

Note: For legacy platform code, several defaults were hardcoded. A Pi with old firmware (firmware older than the model release date) will often assume that it is a Pi 1 Model B and as such up to three output pins may be driven as outputs on boot. See the following section for the GPIOs that may be driven as output by default and the recommended method of designing hardware to guard against this.

After the GPIOMAN/platform code stage the VC bootloader will attempt to probe for an EEPROM attached to GPIO0 and GPIO1. If successful, a GPIO setup map (as described in the EEPROM data format) will be applied to Bank 0 pins. 

Note: For newer firmware, a config.txt option exists to enable the UART on GPIO14/GPIO15 prior to booting the ARM. The EEPROM probe routine will override this behaviour if an EEPROM is found.

**At this point the ARM is booted.** Linux now has notional exclusive control over the settings for Bank 0 pins (except GPIO0 ALT0 and GPIO1 ALT0) from this point onward.

### GPIO Requirements

GPIO pins ID_SC and ID_SD (GPIO0 and GPIO1) are reserved for use solely for board detection / identification. **The only allowed connections to the ID_ pins are an ID EEPROM plus 3.9K pull up resistors. Do not connect anything else to these pins!**

Raspberry Pi models A and B use some bank 0 GPIOs for board control functions and UART output:

    GPIO6 -> LAN_RUN
    GPIO14 -> UART_TX
    GPIO16 -> STATUS_LED

**If a user boots a Pi with legacy firmware these pins may get driven so an add-on board must avoid driving these, or use a current limiting resistor (or some other protection) if that is not possible. Note also that a board must not rely on the pull state of these pins during boot**

## ID EEPROM

Within the set of pins available on the GPIO header, ID_SC and ID_SD (GPIO0/SCL and GPIO1/SDA) are reserved for board detection / identification. **The only allowed connections to the ID_ pins are an ID EEPROM plus 3.9K pull up resistors. Do not connect anything else to these pins!**

The ID EEPROM is interrogated at boot time and provides the Pi with the vendor information, the required GPIO setup (pin settings and functions) for the board as well as a binary Linux device tree fragment which also specifies which hardware is used and therefore which drivers need loading. EEPROM information is also available to userland Linux software for identifying attached boards (probably via a sysfs interface but this is TBD).

Pull-ups must be provided on the top board for ID_SC and ID_SD (SCL and SDA respectively) to 3V3 if using an EEPROM. The required pull-up value is 3.9K.

**EEPROM Device Specification**

- 24Cxx type 3.3V I2C EEPROM must be used (some types are 5V only, do not use these).
- The EEPROM must be of the **16-bit** addressable type (**do not use ones with 8-bit addressing**)
- Do not use 'paged' type EEPROMs where the I2C lower address bit(s) select the EEPROM page.
- Only required to support 100kHz I2C mode.
- Devices that perform I2C clock stretching are not supported.
- Write protect pin must be supported and protect the entire device memory.

Note that due to the restrictions above (only using non-paged 16-bit addressable devices is allowed), many of the smaller I2C EEPROMs are ruled out - please check datasheets carefully when choosing a suitable EEPROM for your HAT.

A recommended part that satisfies the above constraints is OnSemi CAT24C32 which is a 32kbit (4kbyte) device. The minimum EEPROM size required is variable and depends on the size of the vendor data strings in the EEPROM and whether a device tree data blob is included (and its size) and whether any other vendor specific data is included.

It is recommended that EEPROM WP (write protect) pin be connected to a test point on the board and pulled up to 3V3 with a 1K resistor. The idea is that at board test/probe the EEPROM can be written (WP pin can be driven LOW), but there is no danger of a user accidentally changing the device contents once the board leaves the factory. Note that the recommended device has an internal pull down hence the stiff (1K) pull up is required. Note that on some devices WP does not write protect the entire array (e.g. some Microchip variants) – avoid using these.

It may be desirable for a board to have the ability for its EEPROM to be reflashed by an end user, in this case it is recommended to also include a user settable jumper to short WP to GND and make the EEPROM writable once more. At least this way a user has to perform a specific action to make the EEPROM writeable again before being able to re-flash it and a suitable warning process can be put in place to make sure the correct image is used.

Address pins where present on a device should be set to make sure the EEPROM I2C address is 0x50. This usually means tying them all to zero. (NB reduced pin count variants of the recommended device – e.g. SOT23-5 package - usually have A[2:0] set to 0 anyway).

Details of the EEPROM data format can be found in the [EEPROM format specification](eeprom-format.md). [Software tools](./eepromutils) are available for creation of valid EEPROM images, to flash an image or read and dump and image to/from an attached ID EEPROM.

[The following schematic fragment](eeprom-circuit.png) is an example of connecting an EEPROM  including a jumper and probe point to disable write protect.

## Mechanical Specification

The [following drawing](hat-board-mechanical.pdf) gives the mechanical details for add-on boards which conform to the HAT specification.

## Back Powering the Pi via the GPIO Header

It is possible to power the Pi by supplying 5V through the GPIO header pins 2,4 and GND. The acceptable input voltage range is 5V ±5%.

Raspberry Pi Model A+, B+, Raspberry Pi 2B and 3B have an 'ideal' reverse current blocking diode (ZVD) circuit on their 5V input. The 5V GPIO header pins connect to the 5V net after the micro-USB input, polyfuse and input 'ideal' diode [made up of the PFET and matched PNP transistors](zvd-circuit.png). The ideal diode stops any appreciable current flowing back out of the 5V micro USB should the 5V net on the board be at a higher voltage than the 5V micro USB input.

If the add-on board uses any more GPIO connector pins than the first 26 and provides back-powering via the 5V GPIO header pins it is required to:

1. Implement a duplicate power safety diode before the HAT 5V net (which then feeds power back through the 5V GPIO pins) as shown in [this diagram](backpowering-diagram.png). Alternatively provide some other mechanism to guarantee that it is safe if both the Pi PSU and add-on board PSU are connected. It is still recommended to add this circuitry for new board designs that only implement the first 26 pins of the GPIO header but that also implement back powering (see below note)
2. Make sure that the supply that does the back-powering can supply 5V at 2.5A.

NOTE that the Raspberry Pi 3B+ and Pi Zero and ZeroW **do not** include an input ZVD. The micro-USB input on a Pi is expected to / almost universally is driven by a power source which does not sink current, i.e. it will not try to actively pull down a voltage higher than its regulated voltage. If a HAT back-powers a Pi and uses a power source that does not try to sink current (and will safely stop/pause regulation if its input voltage is higher than its regulation voltage) it is OK to not include a ZVD on a HAT. If you are unsure or don't know then please include the ZVD! The HAT designer is responsible for the safety of their product.

**Under no circumstances should a power source be connected to the GPIO header 3.3V pins.**
