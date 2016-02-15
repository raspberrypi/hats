# ADD-ON BOARDS AND HATs

**NOTE** All references to GPIO numbers within this document are referring to the BCM2835 GPIOs (**NOT** pin numbers on the Pi GPIO header).

## Introduction

The Raspberry Pi has been designed specifically with add-on boards in mind. In 2014 after the release of the Raspberry Pi B+, we introduced "HATs" (Hardware Attached on Top). A HAT is any add-on board for Raspberry Pi that conforms to the HAT specification. HATs are compatible with any of the 40 pin Raspbery Pi's but are not backward compatible with models A and B as they use the old 26 pin headers.

There are obviously a lot of add-on boards designed for the original model A and B boards. On the 40 pin GPIO header, the first 26 pins are identical to those of the original models, so most existing boards will still work.

As well as now being a 40W GPIO header (originals on B and A are 26W) the biggest change with these newer add-on boards are that the 40 pin header has 2 special pins (ID_SC and ID_SD) which are reserved exclusively for attaching an "ID EEPROM". The ID EEPROM contains data to identify the board, tells the Pi how the GPIOs need to be set up and what hardware is on the board. This allows the add-on board to be automatically identified and set up by the Pi software at boot time including loading all the necessary drivers.

While we cannot force anyone to follow our minimum requirements or HAT specification, doing so will make users lives easier, safer, and will make us more likely to recommend a product. Likewise if one of the minimum requirements is ignored we are unlikely to look on a product very favourably.

So why are we bothering with all this? Basically we want to ensure consistency and compatibility with future add-on boards, and to allow a much better end-user experience, especially for the less technically aware users.

Finally if you have any questions please head over to the [forums](http://www.raspberrypi.org/forums/viewforum.php?f=100) to ask them.

## New add-on boards basic requirements

If you are designing a new add-on board that takes advantage of the pins on the new GPIO header **other than the original 26** then you **must** follow the basic requirements:

1. The ID_SC and ID_SD pins must only be used for attaching a compatible ID EEPROM. **Do not use ID_SC and ID_SD pins for anything except connecting an ID EEPROM, if unused these pins must be left unconnected**
2. If back-powering via the 5V GPIO header pins you must make sure that it is safe to do so even if the Pi 5V supply is also connected. Adding an ideal 'safety' diode identical to the one on the B+ as per the relevant section of the [design guide](designguide.md) is the recommended way to do this.
3. The board must protect against old firmware accidentally driving GPIO6,14,16 at boot time if any of those pins are also driven by the board itself.

Note that for new designs that only use the original 26 way GPIO header pins it is still recommended to follow requirement 2. if the board supports back-powering a Pi.

## **Minimum** HAT requirements

A board can only be called a HAT if:

1. It conforms to the basic add-on board requirements
2. It has a valid ID EEPROM (including vendor info, GPIO map and valid device tree information).
3. It has a full size 40W GPIO connector.
4. It follows the HAT [mechanical specification](hat-board-mechanical.pdf)
5. It uses a GPIO connector that spaces the HAT between 10mm and 12mm from the Pi (i.e. uses spacers between 10mm and 12mm).
6. If back powering via the GPIO connector the HAT must be able to supply a minimum of 1.3A continuously to the Pi (but ability to supply 2A continuously recommended).

Of course users are free to put an ID EEPROM on boards that don't otherwise conform to the remainder of the specifications - in fact we strongly encourage this; we just want things called HATs to be a known and well-specified entity to make life easier for customers, particularly the less technical ones.

## Design Resources

Before designing any new add-on board (HAT compliant or not) please read the [design guide](designguide.md) carefully.

For what to flash into the ID EEPROM see the [ID EEPROM data format spec](eeprom-format.md).

There are tools and documentation on how to flash ID EEPROMs [here](./eepromutils).

## FAQ

**Q: I want to keep shipping an existing board / ship a new board that only connects to the original 26W GPIO pins.**

This is OK. You can't call it a HAT.
If the board will back-power the Pi we recommend adding the safety diode as per requirement 2. of the basic add-on board requirements.

**Q: I want to ship a board that attaches to the new 40W GPIO header and covers ID_SD and ID_SC but does not include an EEPROM.**

This is OK as long as it meets the basic requirements. You can't call it a HAT.

**Q: I want to ship a board that has an ID EEPROM but does not conform to the remaining HAT specs.**

This is OK as long as it also meets the basic requirements. You can't call it a HAT but you **can** say it supports GPIO autoconfiguration if the EEPROM contains valid vendor, GPIO map and DT blob information.

**Q: I want to ship a HAT but the software for creating the EEPROM and/or DT blob isn't ready yet.**

In this case please ship your board with either a blank EEPROM or one with just the vendor info and GPIO map programmed. Use a 32kbit or larger EEPROM which should be ample for re-flashing later with an image containing a DT blob. You will need to add some ability for a user to un-write-protect the EEPROM to (re-)flash it themselves later such as suggested in the [design guide](designguide.md). Please provide instructions on your website for how to reflash the board when an image becomes available.

**Q: I'm using the HAT mechanical spec but don't want to / can't add the cutout / slot for the display / camera flex.**

This is OK and the board still conforms to the HAT specification. Some HATs will not be able to support the slot/cutout based on where the connectors and components must be placed (but it is recommended to support them if at all possible).

**Q: I want to create a board that connects to the new 'RUN' header.**

No problem but you can't call it a HAT.
HATs are designed to be easy to use. Using the RUN pin requires a user to solder a header onto the Pi hence this is not something we wish to include in the HAT spec.