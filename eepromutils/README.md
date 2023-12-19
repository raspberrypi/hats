# Utilities to create, flash and dump HAT EEPROM images.

## N.B. Like the rest of this repo, these tools have been deprecated. The new HAT+ EEPROM utilities can be found in our [utils repo](https://github.com/raspberrypi/utils/tree/master/eeptools). ##

There is a complete, worked example at:
https://www.raspberrypi.org/forums/viewtopic.php?t=108134

## Usage

1. Create tools with `make && sudo make install`
	Tools available:
	* `eepmake`: Parses EEPROM text file and creates binary `.eep` file
	* `eepdump`: Dumps a binary .eep file as human readable text (for debug)
	* `eepflash`: Write or read .eep binary image to/from HAT EEPROM

2. Edit `eeprom_setting.txt` to suit your specific HAT.

3. Run `./eepmake eeprom_settings.txt eeprom.eep` to create the eep binary

#### On the Raspberry Pi
0. create tools with `make && sudo make install` (If you did the previous steps on your Pi, you don't need to do this).
1. Disable EEPROM write protection
	* Sometimes this requires a jumper on the board
	* Sometimes this is a GPIO
	* Check your schematics
2. Make sure you can talk to the EEPROM
	* In the HAT specification, the HAT EEPROM is connected to pins that can be driven by I2C0.
	  However, this is the same interface as used by the camera and displays, so use of it by the ARMs is discouraged.
	  The `eepflash.sh` script gets around this problem by instantiating a software driven I2C interface using those
	  pins as GPIOs, calling it `i2c-9`:
	```
	   sudo dtoverlay i2c-gpio i2c_gpio_sda=0 i2c_gpio_scl=1 bus=9
	```
	* Install i2cdetect `sudo apt install i2c-tools`
	* Check with `i2cdetect -y 9` (should be at address 0x50)
	```bash
	   i2cdetect -y 9 0x50 0x50
	       0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	   00: 
	   10:
	   20:
	   30:
	   40:
	   50: 50
	   60: 
	   70:
	```
	Normally, you can skip this step, and assume things are working.
3. Flash eep file `sudo ./eepflash.sh -w -t=24c32 -f=eeprom.eep`
4. Enable EEPROM write protection, by undoing step 1 (putting back jumper, or resetting GPIO)

