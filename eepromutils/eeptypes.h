/* Atom types */
#define ATOM_INVALID 	0x0000
#define ATOM_VENDOR_INFO 0x0001
#define ATOM_GPIO_MAP	0x0002
#define ATOM_DT 		0x0003
#define ATOM_CUSTOM		0x0004
#define ATOM_HINVALID	0xffff
//minimal sizes of data structures
#define HEADER_SIZE 12
#define ATOM_SIZE 10
#define VENDOR_SIZE 10
#define GPIO_SIZE 30

#define FORMAT_VERSION 0x01

/* EEPROM header structure */
struct header_t {
	unsigned int signature;
	unsigned char ver;
	unsigned char res;
	unsigned short numatoms;
	unsigned int eeplen;
};

/* Atom structure */
struct atom_t {
	unsigned short type;
	unsigned short count;
	unsigned int dlen;
	char* data;
	unsigned short crc16;
};

/* Vendor info atom data */
struct vendor_info_d {
	unsigned int serial;
	unsigned short pid;
	unsigned short pver;
	unsigned char vslen;
	unsigned char pslen;
	char* vstr;
	char* pstr;
};

/* GPIO map atom data */
struct gpio_map_d {
	unsigned char flags;
	unsigned char bank_drive;
	unsigned char pins[28];
};
