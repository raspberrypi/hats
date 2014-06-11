# Designing Add-on Boards

## Board ID EEPROM:

TODO - spec

## Powering the Pi by supplying 5V from an add-on board:

TODO - use ZVD circuit for safety.


## Non-stackable Boards

Non-stackable boards are defined as boards that, once plugged on to the Pi, do not allow any further add-on boards to attach to the GPIO pins.


## Stackable Boards

Stackable boards are defined as boards that, once plugged on to the Pi, still allow a further add-on board to stack on top of the original and have access to all of the original GPIOs (even if some of these are in fact used by the first stacking board).

Stackable boards present a problem in that different boards may exist that use the same GPIO pin(s). These are termed 'incompatible' boards, and there are rules that must be followed when designing stackable boards to make these incompatible combinations detectable and more importantly make sure stacking incompatible boards does not cause damage to the boards themselves or the Raspberry Pi.

The following rules must be followed when designing a stackable board.

### OUTPUT PINS

  If two incompatible boards are stacked on top of each other, and both use the same GPIO pin as an output then there is a problem as both boards will be driving against each other with the potential for damage. Therefore there are special rules for output pins on boards that can stack.
  
  If a pin is an output but at power on it is Hi-Z or an input with pullup/down (before some software then changes its state) this is OK and the output can be directly connected to the GPIO, otherwise please follow the below rule.
  
  An output must either:
    1. Drive to the same level (Hi or Low) as the pull on the pin, and with output current limited to 20mA maximum via a series resistor (or equivalent).
    2. If (1) cannot be followed, an output must be disconnected from pins at power on (e.g. using an analogue switch or a tristate buffer) and enabled later via software control.

### BIDIR PINS
  
  At power on these must be inputs. Note that a weak pull on a bidir pin is allowed **but only if it is pulling in the same direction as the default power-on GPIO pull.**

  An add-on board **must not** provide a pull on a bidir that is opposite to the default GPIO pull, as this can cause an undefined state on the pin, and potentially other stacked boards (most likeley incompatible ones) may rely on the pull state of a pin to hold their output pins in a disabled (i.e. not driving) state.

### INPUT PINS

  The user must be aware that pins will, at power on, be set to the default power on state (pulled high or low) depending on the default pull up/down for that particular GPIO.
  
  An add-on board **must not** provide a pull on an input that is opposite to the default GPIO pull, as this can cause an undefined state on the pin, and potentially other stacked boards (most likeley incompatible ones) may rely on the pull state of a pin to hold their output pins in a disabled (i.e. not driving) state.




