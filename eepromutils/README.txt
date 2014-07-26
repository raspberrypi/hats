Utilities to create, flash and dump HAT EEPROM images.

Edit eeprom_setting.txt for your particular board and run through
eepmake tool, then use eepflash tool to write to attached HAT ID EEPROM

Tools available:

 eepmake: Parses EEPROM text file and creates binary .eep file

 eepdump: Dumps a binary .eep file as human readable text (for debug)

 eepflash: Write or read .eep binary image to/from HAT EEPROM
