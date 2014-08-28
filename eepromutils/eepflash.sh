#!/bin/sh

MODE="NOT_SET"
FILE="NOT_SET"
TYPE="NOT_SET"

usage()
{
	echo "eepflash: Writes or reads .eep binary image to/from HAT EEPROM on a Raspberry Pi"
	echo ""
	echo "./eepflash.sh"
	echo "	-h --help: display this help message"
	echo "	-r --read: read .eep from the eeprom"
	echo "	-w --write: write .eep to the eeprom"
	echo "	-f=file_name --file=file_name: binary .eep file to read to/from"
	echo "	-t=eeprom_type --type=eeprom_type: eeprom type to use"
	echo "		We support the following eeprom types:"
	echo "		-24c32"
	echo "		-24c64"
	echo "		-24c128"
	echo "		-24c256"
	echo "		-24c512"
	echo "		-24c1024"
	echo ""
}

if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi
 
while [ "$1" != "" ]; do
    PARAM=`echo $1 | awk -F= '{print $1}'`
    VALUE=`echo $1 | awk -F= '{print $2}'`
	case $PARAM in
		-h | --help)
			usage
			exit
			;;
		-r | --read)
			MODE="read"
			;;
		-w | --write)
			MODE="write"
			;;
		-t | --type)
			if [ "$VALUE" = "24c32" ] || [ "$VALUE" = "24c64" ] || [ "$VALUE" = "24c128" ] ||
				[ "$VALUE" = "24c256" ] || [ "$VALUE" = "24c512" ] || [ "$VALUE" = "24c1024" ]; then
					TYPE=$VALUE
			else
				echo "ERROR: Unrecognised eeprom type. Try -h for help"
				exit 1
			fi
			;;
		-f | --file)
			FILE=$VALUE
			;;
		*)
			echo "ERROR: unknown parameter \"$PARAM\""
			usage
			exit 1
			;;
    esac
    shift
done
 
if [ "$MODE" = "NOT_SET" ]; then
	echo "You need to set mode (read or write). Try -h for help."
	exit 1
elif [ "$FILE"  = "NOT_SET" ]; then
	echo "You need to set binary .eep file to read to/from. Try -h for help."
	exit 1
elif [ "$TYPE" = "NOT_SET" ]; then
	echo "You need to set eeprom type. Try -h for help."
	exit 1
fi

echo "This will disable the camera so you will need to REBOOT after this process completes."
echo "This will attempt to write to i2c address 0x50. Make sure there is an eeprom at this address."
echo "This script comes with ABSOLUTELY no warranty. Continue only if you know what you are doing."

while true; do
	read -p "Do you wish to continue? (yes/no): " yn
	case $yn in
		yes | Yes ) break;;
		no | No ) exit;;
		* ) echo "Please type yes or no.";;
	esac
done

modprobe i2c-bcm2708
rc=$?
if [ $rc != 0 ]; then
	echo "Modprobe of i2c-bcm2708 failed. Do an rpi-update."
	exit $rc
fi
modprobe at24
rc=$?
if [ $rc != 0 ]; then
	echo "Modprobe of at24 failed. Do an rpi-update."
	exit $rc
fi

if [ ! -d "/sys/class/gpio/gpio28" ]; then
	echo 28 > /sys/class/gpio/export
fi

if [ ! -d "/sys/class/gpio/gpio29" ]; then
	echo 29 > /sys/class/gpio/export
fi

echo in > /sys/class/gpio/gpio28/direction
echo in > /sys/class/gpio/gpio29/direction

if [ ! -d "/sys/class/i2c-adapter/i2c-0/0-0050" ]; then
	echo "$TYPE 0x50" > /sys/class/i2c-adapter/i2c-0/new_device
fi


if [ "$MODE" = "write" ]
 then
	echo "Writing..."
	dd if=$FILE of=/sys/class/i2c-adapter/i2c-0/0-0050/eeprom
	rc=$?
elif [ "$MODE" = "read" ]
 then
	echo "Reading..."
	dd if=/sys/class/i2c-adapter/i2c-0/0-0050/eeprom of=$FILE
	rc=$?
fi

if [ $rc != 0 ]; then
	echo "Error doing I/O operation."
	exit $rc
else
	echo "Done."
fi
