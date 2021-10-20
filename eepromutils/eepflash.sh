#!/bin/sh

me=$(basename "$0")

MODE="NOT_SET"
FILE="NOT_SET"
TYPE="NOT_SET"
BUS="NOT_SET"
ADDR="NOT_SET"
BATCH="NOT_SET"

usage()
{
	echo "eepflash: Writes or reads .eep binary image to/from HAT EEPROM on a Raspberry Pi"
	echo ""
	echo "$me"
	echo "	-h --help: display this help message"
	echo "	-r --read: read .eep from the EEPROM"
	echo "	-w --write: write .eep to the EEPROM"
	echo "	-y --yes: disable interactive mode. By default eepflash.sh will wait"
	echo "	    for confirmation from the user. When this flag is used, it will"
	echo "	    perform the operation directly. This is mainly meant to be used"
	echo "	    in scripts"
	echo "	-f=file_name --file=file_name: binary .eep file to read to/from"
	echo "	    N.B. -f file_name and --file file_name (without =) also accepted"
	echo "	-d= --device= i2c bus number (ex if the eeprom is on i2c-0 set -d=0)"
	echo "	-a= --address= i2c EEPROM address"
	echo "	-t=eeprom_type --type=eeprom_type: EEPROM type to use"
	echo "		We support the following EEPROM types:"
	echo "			24c32"
	echo "			24c64"
	echo "			24c128"
	echo "			24c256"
	echo "			24c512"
	echo "			24c1024"
	echo ""
	echo "Example:"
	echo "$me -w -f=crex0.1.eep -t=24c32 -d=1 -a=57"
	echo "$me -r -f dump.eep -t=24c32 -d=1 -a=57"
	echo ""
}

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
		-y | --yes)
			BATCH="yes"
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
		-d | --device)
			BUS=$VALUE
			;;
		-a | --address)
			ADDR=$VALUE
			;;
		-f | --file)
			FILE=$VALUE
			if [ "$1" = "-f" -o "$1" = "--file" ]; then
				shift
				FILE=$1
			fi
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

if [ "$ADDR" = "NOT_SET" ]; then
	ADDR=50
fi

echo "This will attempt to talk to an eeprom at i2c address 0x$ADDR on bus $BUS. Make sure there is an eeprom at this address."
echo "This script comes with ABSOLUTELY no warranty. Continue only if you know what you are doing."

if [ "$(id -u)" != "0" ]; then
	echo "This script must be run as root" 1>&2
	exit 1
fi

if [ "$BATCH" = "NOT_SET" ]; then
	while true; do
		read -p "Do you wish to continue? (yes/no): " yn
		case $yn in
			yes | Yes ) break;;
			no | No ) exit;;
			* ) echo "Please type yes or no.";;
		esac
	done
fi

modprobe i2c_dev
if [ "$BUS" = "NOT_SET" ]; then
	if [ -e "/dev/i2c-0" ]; then
		BUS=0
	elif [ -e "/dev/i2c-9" ]; then
		BUS=9
	else
		dtoverlay i2c-gpio i2c_gpio_sda=0 i2c_gpio_scl=1 bus=9
		rc=$?
		if [ $rc != 0 ]; then
			echo "Loading of i2c-gpio dtoverlay failed."
			exit $rc
		fi
		if [ -e "/dev/i2c-9" ]; then
			BUS=9
		else
			echo "Expected I2C bus (i2c-9) not found."
		fi
	fi
fi

modprobe at24

rc=$?
if [ $rc != 0 ]; then
	echo "Modprobe of at24 failed."
	exit $rc
fi

SYS=/sys/class/i2c-adapter/i2c-$BUS

if [ ! -d "$SYS/$BUS-00$ADDR" ]; then
	echo "$TYPE 0x$ADDR" > $SYS/new_device
fi

DD_VERSION=$(dd --version 2>&1 | grep coreutils | sed -e 's/\.//' | cut -d' ' -f 3)
if [ -z "$DD_VERSION" ]; then
	# probably busybox's dd
	DD_STATUS=""
else
	if [ $DD_VERSION -ge 824 ]; then
		DD_STATUS="status=progress"
	else
		DD_STATUS="status=none"
	fi
fi

if [ "$MODE" = "write" ]
 then
	echo "Writing..."
	dd if="$FILE" of=$SYS/$BUS-00$ADDR/eeprom $DD_STATUS
	rc=$?
elif [ "$MODE" = "read" ]
 then
	echo "Reading..."
	dd if=$SYS/$BUS-00$ADDR/eeprom of="$FILE" $DD_STATUS
	rc=$?
fi

echo "Closing EEPROM Device."
echo "0x$ADDR" > $SYS/delete_device

if [ $rc != 0 ]; then
	echo "Error doing I/O operation."
	exit $rc
else
	echo "Done."
fi
