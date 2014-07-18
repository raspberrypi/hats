# B+ HAT DESIGN GUIDE

**NOTE THIS INFORMATION IS CURRENTLY STILL CHANGING**

## Introduction

The Raspberry Pi B+ has been designed specifically with add-on boards - we're calling them Raspberry Pi HATs (Hardware Attached on Top) - in mind. This guide contains both recommendataions and also **requirements** that must be followed when designing an add-on board for the B+.

**If you are thinking about or are designing an add-on board for the Raspberry Pi B+ please read and follow this design guide carefully and make sure the requirements are met. There aren't too many hard requirements, and they shouldn't be too dificult to design for or follow.**

While we cannot force anyone to follow this guide, any boards which break the rules (and therefore may cause incompatibility or issues for end users) will be thoroughly denounced by the Foundation (and generally moaned about in the forums). You have been warned.

So why are we bothering with all this? Basically we want to ensure consistency and compatibility with future add-on boards, and to allow a much better end-user experience, especially for the less technically aware users.

Finally if you have any questions please head over to the [forums](http://www.raspberrypi.org/forums/viewforum.php?f=100) to ask them.

## Types of HATs

There are 2 basic types of HATs, stackable and non-stackable.

Stackable HATs are defined as boards that, once plugged on to the Pi, still allow a further add-on board to stack on top of the original and have access to all of the original GPIOs (even if some of these are in fact used by the first stacking board).

Non-stackable boards are defined as boards that, once plugged on to the Pi, do not allow any further add-on boards to attach directly to the GPIO pins.

### Stackable Boards

Stackable boards present a problem in that different boards may exist that use the same GPIO pin(s). These are termed 'incompatible' boards, and there are rules that must be followed when designing stackable boards to make these incompatible combinations detectable and more importantly make sure stacking incompatible boards does not cause damage to the boards themselves or to the Raspberry Pi.

The following rules **must** be followed when designing a stackable board.

#### GPIO Pin Usage Rules for Stackable Boards

Note that after power-on the bank 0 GPIOs on GPIO header J8 (except ID_SD and ID_SC which are GPIO0 and 1 respectiveley) will be inputs with either a pull up or pull down. The default pull state can be found in the BCM2835 peripherals specificaion[] section 6.2 table 6-31 (see the "Pull" column).

**Output Pins**

If two incompatible boards are stacked on top of each other, and both use the same GPIO pin as an output then there is a problem as both boards will be driving against each other with the potential for damage. Therefore there are special rules for output pins on boards that can stack.

If a pin is an output but at power on it is Hi-Z or an input with pullup/down (before some software then changes its state) this is OK and the output can be directly connected to the GPIO, otherwise please follow the below rule.

An output must either:
  1. Drive to the same level (Hi or Low) as the pull on the pin, and with output current limited to 20mA maximum via a series resistor (or equivalent).
  2. If (1.) cannot be followed, an output must be disconnected from pins at power on (e.g. using an analogue switch or a tristate buffer) and enabled later via software control.

**Bidirectional Pins**

At power-on these must be inputs. Note that a weak pull on a bidirectional pin is allowed **but only if it is pulling in the same direction as the default power-on GPIO pull.**

An add-on board **must not** provide a pull on a bidirectional pin that is opposite to the default GPIO pull, as this can cause an undefined state on the pin, and potentially other stacked boards (most likeley incompatible ones) may rely on the pull state of a pin to hold their output pins in a disabled (i.e. not driving) state.

**Input Pins**

The user must be aware that pins will, at power on, be set to the default power on state (pulled high or low) depending on the default pull up/down for that particular GPIO.

An add-on board **must not** provide a pull on an input that is opposite to the default GPIO pull, as this can cause an undefined state on the pin, and potentially other stacked boards (most likeley incompatible ones) may rely on the pull state of a pin to hold their output pins in a disabled (i.e. not driving) state.

#### ID EEPROM Rules for Stackable Boards

If a board is stackable it is **required** to include an ID EEPROM, as well as the ability for a user (via board jumpers or DIP switches) to set the ID EEPROM address to be one of 4 possible values (A0 and A1 address bits of EEPROM must be user settable).

### Non-stackable Boards

As non-stackable boards have exclusive use of the GPIO pins, they don't have to follow any specifc rules for GPIO usage and allocation (as compared to stackable boards), though common sense and good design practice should apply.

It is strongly recommended to include an ID EEPROM on non-stackable boards but this is not a hard requirement. The ID EEPROM address pins should all be set to zero in this case.

## ID EEPROM

The ID EEPROM is interrogated at boot time and provides the Pi with the required GPIO setup (pin settings and functions) for the HAT as well as (optionally) a Linux device tree fragment which also specifies which hardware is used and therefore which drivers need loading. EEPROM information is also available to userland Linux software for identifying attached boards.

Within the set of pins available on the J8 GPIO header, ID_SC and ID_SD (GPIO0 and GPIO1) are reserved for use solely for board identification. **An I2C EEPROM plus pull-up resistors should be the only connections to these pins.**

Pull-ups must be provided on the top board for SCL and SDA  (ID_SC and ID_SD respectively) to 3V3 – recommended pull-up value is 2.2K.

A 24Cxx type I2C EEPROM must be used. The device must be powered from 3V3 (avoid the 5V only variants). A recommended part is OnSemi CAT24Cxx, for example CATC24C32 for a 32kbit device. The minimum EEPROM size required is variable and depends on the size of the vendor data strings in the EEPROM and whether a device tree data blob is included (and its size) and whether any other vendor specific data is included.

It is recommended that EEPROM WP (write protect) pin be connected to a test point on the board and pulled up to 3V3 with a 1K resistor. The idea is that at board test/probe the EEPROM can be written (WP pin can be driven LOW), but there is no danger of a user accidentally changing the device contents once the board leaves the factory. Note that the recommended device has an internal pull down hence the stiff (1K) pull up is required. Note that on some devices WP does not write protect the entire array (e.g. some Microchip variants) – avoid using these.
It may be desirable for a HAT to have the ability for its EEPROM to be reflashed by an end user, in this case it is recommended to also include a user settable jumper (or dip switch) to short WP to GND and make the EEPROM writable once more. At least this way a user has to perform a specific action to make the EEPROM writeable again before being able to re-flash it and a suitable warning process can be put in place to make sure the correct image is used.

For top boards that stack, jumpers or DIP switches should be provided to adjust A0 and A1 with A2 being fixed to zero.

For top boards that don’t stack, set all address bits to zero. (NB reduced pin count variants of the recommended device – e.g. SOT23-5 package - usually have A[2:0] set to 0 anyway but please check).

Implementing an EEPROM is optional for non-stackable boards but mandatory for stackable boards. **Implementing an EEPROM is also a prerequisite for support in the default kernel.**

Details of the EEPROM format can be found in the [EEPROM format specification](eeprom-format.md). [Software tools] are available for creation of valid EEPROM images, to flash an image or read and dump and image to/from an attached HAT EEPROM.

The following is an example of connecting an EEPROM on a stackable HAT and includes jumpers for address setting as well as a jumper and probe point to disable write protect. [PIC]

## Mechanical Specification

TODO

## Connectors etc.

TODO

## Back Powering the Pi via the J8 GPIO Header

It is possible to power the Pi by supplying 5V thourhg the GPIO header (J8) pins 2,4 and GND. The acceptable input voltage range is 5V ±5%.

On the Pi, the 5V GPIO header pins connect to the 5V net after the micro-USB input, polyfuse and input 'ideal' power ORing diode (made up of the PFET and matched PNP transistors). 

If the Pi is going to be back-powered via the 5V GPIO header pins it is **required** to implement a duplicate power ORing diode to feed the HAT 5V net (and therefore Pi 5V, via the GPIO header pins).

Implementing the back-powering this way makes it completely safe to provide power to the Pi from the HAT board but also have the Pi 5V power supply attached.

For clarity, the following diagram shows the required HAT back-powering arrangement using the ideal diodes:
[PIC]

**Under no circumstances should a power source be connected to the J8 3.3V pins.**
