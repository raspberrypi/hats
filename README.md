# Designing Add-on Boards

## Board ID EEPROM:

Within the set of pins available on the P1 GPIO header, GPIO0 and GPIO1 are reserved for use solely for board identification. An I2C EEPROM plus pull-up resistors should be the only connections to these pins.

The EEPROM contains two elements: a fixed-size GPIO usage mapping and an optional Devicetree fragment that allows for automatic configuration of pins and loading of drivers.

Implementing an EEPROM is optional for non-stackable boards but mandatory for stackable boards. Implementing an EEPROM is also a prerequisite for support in the default kernel. 

Details of the GPIO mapping and devicetree fragment can be found on the [EEPROM Specification](eeprom.md) page.

## Powering the Pi by supplying 5V from an add-on board:

It is possible to power the Pi via P1 pins 2+4 and GND. The acceptable input voltage range is 5V &plusmn;5%.

The 5V pins connect to the 5V net after the micro-USB polyfuse and ideal diode, therefore if the Pi is going to be powered via the 5V pins it is strongly recommended to implement a duplicate of F1 / Q3 / U14 / R2 / R3 on the add-on board.

Under no circumstances should a power source be connected to the 3.3V pins on P1.

## Non-stackable Boards

Non-stackable boards are defined as boards that, once plugged on to the Pi, do not allow any further add-on boards to attach to the GPIO pins.

## Stackable Boards

Stackable boards are defined as boards that, once plugged on to the Pi, still allow a further add-on board to stack on top of the original and have access to all of the original GPIOs (even if some of these are in fact used by the first stacking board).

Stackable boards present a problem in that different boards may exist that use the same GPIO pin(s). These are termed 'incompatible' boards, and there are rules that must be followed when designing stackable boards to make these incompatible combinations detectable and more importantly make sure stacking incompatible boards does not cause damage to the boards themselves or the Raspberry Pi.

The following rules must be followed when designing a stackable board.

### Output Pins

  If two incompatible boards are stacked on top of each other, and both use the same GPIO pin as an output then there is a problem as both boards will be driving against each other with the potential for damage. Therefore there are special rules for output pins on boards that can stack.
  
  If a pin is an output but at power on it is Hi-Z or an input with pullup/down (before some software then changes its state) this is OK and the output can be directly connected to the GPIO, otherwise please follow the below rule.
  
  An output must either:
    1. Drive to the same level (Hi or Low) as the pull on the pin, and with output current limited to 20mA maximum via a series resistor (or equivalent).
    2. If (1) cannot be followed, an output must be disconnected from pins at power on (e.g. using an analogue switch or a tristate buffer) and enabled later via software control.

### Bidirectional Pins
  
  At power-on these must be inputs. Note that a weak pull on a bidirectional pin is allowed **but only if it is pulling in the same direction as the default power-on GPIO pull.**

  An add-on board **must not** provide a pull on a bidirectional pin that is opposite to the default GPIO pull, as this can cause an undefined state on the pin, and potentially other stacked boards (most likeley incompatible ones) may rely on the pull state of a pin to hold their output pins in a disabled (i.e. not driving) state.

### Input Pins

  The user must be aware that pins will, at power on, be set to the default power on state (pulled high or low) depending on the default pull up/down for that particular GPIO.
  
  An add-on board **must not** provide a pull on an input that is opposite to the default GPIO pull, as this can cause an undefined state on the pin, and potentially other stacked boards (most likeley incompatible ones) may rely on the pull state of a pin to hold their output pins in a disabled (i.e. not driving) state.
