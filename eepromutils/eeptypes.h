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

#define CRC16 0x8005

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


unsigned short getcrc(char* data, unsigned int size) {
	
	unsigned short out = 0;
    int bits_read = 0, bit_flag;
	
    /* Sanity check: */
    if((data == NULL) || size==0)
        return 0;

    while(size > 0)
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

        /* Increment bit counter: */
        bits_read++;
        if(bits_read > 7)
        {
			bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if(bit_flag)
            out ^= CRC16;

    }

    // item b) "push out" the last 16 bits
    int i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if(bit_flag)
            out ^= CRC16;
    }

    // item c) reverse the bits
    unsigned short crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out) crc |= j;
    }
	
    return crc;
}
