# B+ HAT ID EEPROM DATA SPECIFICATION

**CURRENTLY THIS SPEC IS PRELIMINARY AND THEREFORE STILL LIKELEY TO CHANGE**

Note that there are [software tools] for creation of EEPROM images and for flashing / dumping images from attached HATs.

## EEPROM Structure

```
  HEADER  <- EEPROM header (Required)
  ATOM1   <- Vendor info atom (Required)
  ATOM2   <- GPIO map atom (Required)
  ATOM3   <- DT blob atom (Recommended)
  ...
  ATOMn
```

## EEPROM Header Structure

```
  Bytes   Field
  4       signature   signature: 0x52, 0x2D, 0x50, 0x69 ("R-Pi" in ASCII)
  1       version     EEPROM data format version (0x00 reserved, 0x01 = first version)
  1       reserved    set to 0
  2       numatoms    total atoms in EEPROM
```

## Atom Structure
```
  Bytes   Field
  2       type        atom type
  2       count       incrementing atom count
  4       dlen        length in bytes of data+CRC
  N       data        N bytes, N = dlen-2
  2       crc16       CRC-16-CCITT of entire atom (type, count, dlen, data)
```

## Atom Types

```
  0x0000 = invalid
  0x0001 = vendor info
  0x0002 = GPIO map
  0x0003 = Linux device tree blob
  0x0004 = manufacturer custom data
  0x0005-0xffff = reserved for future use
  0xffff = invalid
```

### Vendor info atom data (type=0x0001):

```
  Bytes   Field
  4       serial      product serial number
  2       pid         product ID
  2       pver        product version
  1       vslen       vendor string length (bytes)
  1       pslen       product string length (bytes)
  X       vstr        ASCII vendor string e.g. "ACME Technology Company"
  Y       pstr        ASCII product string e.g. "Special Sensor Board"
```

### GPIO map atom data (type=0x0002):

  GPIO map for bank 0 GPIO on 40W B+ header.

```
  Bytes   Field
  1       flags
            Bits in Byte:
            [  0] can_stack   1=board designed to be stackable, 0=board not intended to stack
            [7:2] reserved    set to 0
  1       bank_drive  bank drive strength/slew/hysteresis, BCM2835 can only set per bank, not per IO
            Bits in byte:
            [  0] nondefault  1=non-default pad drive/slew/hyst (**recommended to leave this bit at zero!**)
            [  1] reserved    set to 0
            [  2] noslew      0=slew rate limited 1=slew rate not limited
            [  3] hysten      0=hysteresis disabled, 1=hysteresis enabled
            [6:4] drive       pad drive strength in mA 0-7=2,4,6,8,10,12,14,16mA
            [  7] reserved    set to 0
  28      1 byte per IO pin
            Bits in each byte:
            [  0] is_used     1=board uses this pin, 0=not connected and therefore not used
            [3:1] func_sel    GPIO alt FN as per BCM2835 datasheet
            [5:4] iotype      00=input, 01=output, 10=bidir,    11=open drain
            [7:6] pulltype    00=none,  01=pullup, 10=pulldown, 11=leave at default setting
```

### Device tree atom data (type=0x0003):

Binary DT blob fragment for board hardware.

