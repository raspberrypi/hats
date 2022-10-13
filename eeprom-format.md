# B+ HAT ID EEPROM FORMAT SPECIFICATION

**CURRENTLY THIS SPEC IS PRELIMINARY AND THEREFORE STILL LIKELEY TO CHANGE**

Note that there are [software tools](./eepromutils) for creation of EEPROM images and for flashing / dumping images from attached HATs.

## EEPROM Structure

```
  HEADER  <- EEPROM header (Required)
  ATOM1   <- Vendor info atom (Required)
  ATOM2   <- GPIO (bank 0) map atom (Required)
  ATOM3   <- DT blob atom (Required for compliance with the HAT specification)
  ...
  ATOMn-1
  ATOMn   <- GPIO (bank 1) map atom (Optional, only available on CM1, CM3, CM3+ and CM4S)
```

## EEPROM Header Structure

```
  Bytes   Field
  4       signature   signature: 0x52, 0x2D, 0x50, 0x69 ("R-Pi" in ASCII)
  1       version     EEPROM data format version (0x00 reserved, 0x01 = first version)
  1       reserved    set to 0
  2       numatoms    total atoms in EEPROM
  4       eeplen      total length in bytes of all eeprom data (including this header)
```

## Atom Structure
```
  Bytes   Field
  2       type        atom type
  2       count       incrementing atom count
  4       dlen        length in bytes of data+CRC
  N       data        N bytes, N = dlen-2
  2       crc16       CRC-16 of entire atom (type, count, dlen, data)
```

## Atom Types

```
  0x0000 = invalid
  0x0001 = vendor info
  0x0002 = GPIO (bank 0) map
  0x0003 = Linux device tree blob
  0x0004 = manufacturer custom data
  0x0005 = GPIO (bank 1) map
  0x0006-0xfffe = reserved for future use
  0xffff = invalid
```

### Vendor info atom data (type=0x0001):

Note that the UUID is mandatory and must be filled in correctly according to RFC 4122
(every HAT can then be uniquely identified). It protects against the case where a user 
accidentally stacks 2 identical HATs on top of each other - this error case is only 
detectable if the EEPROM data in each is different. The UUID is also useful for 
manufacturers as a per-board 'serial number'.

```
  Bytes   Field
  16      uuid        UUID (unique for every single board ever made)
  2       pid         product ID
  2       pver        product version
  1       vslen       vendor string length (bytes)
  1       pslen       product string length (bytes)
  X       vstr        ASCII vendor string e.g. "ACME Technology Company"
  Y       pstr        ASCII product string e.g. "Special Sensor Board"
```

### GPIO (bank 0) map atom data (type=0x0002):

  GPIO map for bank 0 GPIO on 40W B+ header.

  **NOTE** GPIO number refers to BCM2835 GPIO number and **NOT** J8 pin number!

```
  Bytes   Field
  1       bank_drive  bank drive strength/slew/hysteresis, BCM2835 can only set per bank, not per IO
            Bits in byte:
            [3:0] drive       0=leave at default, 1-8=drive*2mA, 9-15=reserved
            [5:4] slew        0=leave at default, 1=slew rate limiting, 2=no slew limiting, 3=reserved
            [7:6] hysteresis  0=leave at default, 1=hysteresis disabled, 2=hysteresis enabled, 3=reserved
  1       power
            [1:0] back_power  0=board does not back power Pi
                              1=board back powers and can supply up to 1.3A to the Pi
                              2=board back powers and can supply up to 2A to the Pi
                              3=reserved
                              If back_power=2 high current USB mode is automatically enabled.
            [7:2] reserved    set to 0
  28      1 byte per IO pin
            Bits in each byte:
            [2:0] func_sel    GPIO function as per FSEL GPIO register field in BCM2835 datasheet
            [4:3] reserved    set to 0
            [6:5] pulltype    0=leave at default setting,  1=pullup, 2=pulldown, 3=no pull
            [  7] is_used     1=board uses this pin, 0=not connected and therefore not used
```

### Device Tree atom data (type=0x0003):

Binary data (the name or contents of a `.dtbo` overlay, for board hardware).

For more information on the Device Tree atom contents, see the [Device Tree Guide](devicetree-guide.md).

### GPIO (bank 1) map atom data (type=0x0005):

  GPIO map for bank 1 GPIOs (28-45) only available on CM1, CM3, CM3+ and CM4S. This must be the last atom.

```
  Bytes   Field
  1       bank_drive  bank drive strength/slew/hysteresis, BCM2835 can only set per bank, not per IO
            Bits in byte:
            [3:0] drive       0=leave at default, 1-8=drive*2mA, 9-15=reserved
            [5:4] slew        0=leave at default, 1=slew rate limiting, 2=no slew limiting, 3=reserved
            [7:6] hysteresis  0=leave at default, 1=hysteresis disabled, 2=hysteresis enabled, 3=reserved
  1       reserved
            [7:0] reserved    set to 0
  18      1 byte per IO pin
            Bits in each byte:
            [2:0] func_sel    GPIO function as per FSEL GPIO register field in BCM2835 datasheet
            [4:3] reserved    set to 0
            [6:5] pulltype    0=leave at default setting,  1=pullup, 2=pulldown, 3=no pull
            [  7] is_used     1=board uses this pin, 0=not connected and therefore not used
```
