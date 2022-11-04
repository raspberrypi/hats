#include <stdint.h>

/* Atom types */
#define ATOM_INVALID_TYPE   0x0000
#define ATOM_VENDOR_TYPE    0x0001
#define ATOM_GPIO_TYPE      0x0002
#define ATOM_DT_TYPE        0x0003
#define ATOM_CUSTOM_TYPE    0x0004
#define ATOM_GPIO_BANK1_TYPE 0x0005
#define ATOM_HINVALID_TYPE  0xffff

#define ATOM_VENDOR_NUM     0x0000
#define ATOM_GPIO_NUM       0x0001
#define ATOM_DT_NUM         0x0002

//minimal sizes of data structures
#define HEADER_SIZE 12
#define ATOM_SIZE   10
#define VENDOR_SIZE 22
#define GPIO_SIZE   30
#define GPIO_BANK1_SIZE 20
#define CRC_SIZE     2

#define GPIO_MIN     2
#define GPIO_COUNT        28
#define GPIO_COUNT_BANK1  18
#define GPIO_COUNT_TOTAL  (GPIO_COUNT + GPIO_COUNT_BANK1)

#define FORMAT_VERSION 0x01

#define CRC16 0x8005

/* EEPROM header structure */
// Signature is "R-Pi" in ASCII. It is required to reversed (little endian) on disk.
#define HEADER_SIGN be32toh((((char)'R' << 24) | ((char)'-' << 16) | ((char)'P' << 8) | ((char)'i')))

struct header_t {
	uint32_t signature;
	unsigned char ver;
	unsigned char res;
	uint16_t numatoms;
	uint32_t eeplen;
};

/* Atom structure */
struct atom_t {
	uint16_t type;
	uint16_t count;
	uint32_t dlen;
	char* data;
	uint16_t crc16;
};

/* Vendor info atom data */
struct vendor_info_d {
	uint32_t serial[4]; // 0 = least significant, 3 = most significant
	uint16_t pid;
	uint16_t pver;
	unsigned char vslen;
	unsigned char pslen;
	char* vstr;
	char* pstr;
};

/* GPIO map atom data */
struct gpio_map_d {
	unsigned char flags;
	unsigned char power;
	unsigned char pins[GPIO_COUNT];
};

uint16_t getcrc(char* data, unsigned int size)
{
	uint16_t out = 0;
	int bits_read = 0, bit_flag;

	/* Sanity check: */
	if((data == NULL) || size == 0)
		return 0;

	while(size > 0)
	{
		bit_flag = out >> 15;

		/* Get next bit: */
		out <<= 1;
		// item a) work from the least significant bits
		out |= (*data >> bits_read) & 1;

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
	uint16_t crc = 0;
	i = 0x8000;
	int j = 0x0001;
	for (; i != 0; i >>=1, j <<= 1) {
		if (i & out) crc |= j;
	}

	return crc;
}
