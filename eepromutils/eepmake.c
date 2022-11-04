/*
 *	Parses EEPROM text file and createds binary .eep file
 *	Usage: eepmake input_file output_file
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#include "eeptypes.h"

//todo: larger initial mallocs

struct header_t header;
struct atom_t *custom_atom, vinf_atom, gpio_atom, gpio_atom_bank1, dt_atom;
struct vendor_info_d* vinf;

bool product_serial_set, product_id_set, product_ver_set, vendor_set, product_set, 
			gpio_drive_set, gpio_slew_set, gpio_hysteresis_set, gpio_power_set,
			bank1_gpio_drive_set, bank1_gpio_slew_set, bank1_gpio_hysteresis_set;
			
bool data_receive, has_dt, receive_dt, has_bank1;
			
char **data;
char *current_atom; //rearranged to write out
unsigned int data_len, custom_ct, total_size, data_cap, custom_cap;


int write_binary(char* out) {
	FILE *fp;
	int offset;
	unsigned int i;
	short crc;
	
	fp=fopen(out, "wb");
	if (!fp) {
		printf("Error writing file %s\n", out);
		return -1;
	}
	
	fwrite(&header, sizeof(header), 1, fp);
		
		
	current_atom = (char *) malloc(vinf_atom.dlen+ATOM_SIZE-CRC_SIZE);
	offset = 0;
	//vendor information atom first part
	memcpy(current_atom, &vinf_atom, ATOM_SIZE-CRC_SIZE);
	offset += ATOM_SIZE-2;
	//data first part
	memcpy(current_atom+offset, vinf_atom.data, VENDOR_SIZE);
	offset += VENDOR_SIZE;	
	//data strings
	memcpy(current_atom+offset, vinf->vstr, vinf->vslen);
	offset += vinf->vslen;
	memcpy(current_atom+offset, vinf->pstr, vinf->pslen);
	offset += vinf->pslen;
	//vinf last part
	crc = getcrc(current_atom, offset);
	memcpy(current_atom+offset, &crc, CRC_SIZE);
	offset += CRC_SIZE;
	
	fwrite(current_atom, offset, 1, fp);
	free(current_atom);
	
	current_atom = (char *) malloc(gpio_atom.dlen+ATOM_SIZE-CRC_SIZE);
	offset = 0;
	//GPIO map first part
	memcpy(current_atom, &gpio_atom, ATOM_SIZE-CRC_SIZE);
	offset += ATOM_SIZE-CRC_SIZE;
	//GPIO data
	memcpy(current_atom+offset, gpio_atom.data, GPIO_SIZE);
	offset += GPIO_SIZE;
	//GPIO map last part
	crc = getcrc(current_atom, offset);
	memcpy(current_atom+offset, &crc, CRC_SIZE);
	offset += CRC_SIZE;
	
	fwrite(current_atom, offset, 1, fp);
	free(current_atom);
	
	if (has_dt) {
		printf("Writing out DT...\n");
		current_atom = (char *) malloc(dt_atom.dlen+ATOM_SIZE-CRC_SIZE);
		offset = 0;
		
		memcpy(current_atom, &dt_atom, ATOM_SIZE-CRC_SIZE);
		offset += ATOM_SIZE-CRC_SIZE;
		
		memcpy(current_atom+offset, dt_atom.data, dt_atom.dlen-CRC_SIZE);
		offset += dt_atom.dlen-CRC_SIZE;
		
		crc = getcrc(current_atom, offset);
		memcpy(current_atom+offset, &crc, CRC_SIZE);
		offset += CRC_SIZE;
		
		fwrite(current_atom, offset, 1, fp);
		free(current_atom);
	}
	
	for (i = 0; i<custom_ct; i++) {
		custom_atom[i].count-=!has_dt;
		
		current_atom = (char *) malloc(custom_atom[i].dlen+ATOM_SIZE-CRC_SIZE);
		offset = 0;
		
		memcpy(current_atom, &custom_atom[i], ATOM_SIZE-CRC_SIZE);
		offset += ATOM_SIZE-CRC_SIZE;
		
		memcpy(current_atom+offset, custom_atom[i].data, custom_atom[i].dlen-CRC_SIZE);
		offset += custom_atom[i].dlen-CRC_SIZE;
		
		crc = getcrc(current_atom, offset);
		memcpy(current_atom+offset, &crc, CRC_SIZE);
		offset += CRC_SIZE;
		
		fwrite(current_atom, offset, 1, fp);
		free(current_atom);
	}
	
	if (has_bank1) {
		current_atom = (char *) malloc(gpio_atom_bank1.dlen+ATOM_SIZE-CRC_SIZE);
		offset = 0;
		//GPIO map first part
		gpio_atom_bank1.count = 2 + custom_ct;
		if (has_dt)
			gpio_atom_bank1.count++;
		memcpy(current_atom, &gpio_atom_bank1, ATOM_SIZE-CRC_SIZE);
		offset += ATOM_SIZE-CRC_SIZE;
		//GPIO data
		memcpy(current_atom+offset, gpio_atom_bank1.data, GPIO_BANK1_SIZE);
		offset += GPIO_BANK1_SIZE;
		//GPIO map last part
		crc = getcrc(current_atom, offset);
		memcpy(current_atom+offset, &crc, CRC_SIZE);
		offset += CRC_SIZE;

		fwrite(current_atom, offset, 1, fp);
		free(current_atom);
	}
	
	fflush(fp);
	fclose(fp);
	return 0;
}


int parse_data(char* c) {
	int k;
	char* i = c;
	char* j = c;
	while(*j != '\0')
	{
		*i = *j++;
		if(isxdigit(*i))
			i++;
	}
	*i = '\0';
	
	int len = strlen(c);
	if (len % 2 != 0) {
		printf("Error: data must have an even number of hex digits\n");
		return -1;
	} else {
		for (k = 0; k<len/2; k++) {
			int byte;

			if (data_len==data_cap) {
				data_cap *=2;
				*data = (char *) realloc(*data, data_cap);
			}
			
			sscanf(c, "%2x", &byte);
			(*data)[data_len++] = byte;
			c+=2;
		}
	}

	return 0;
}


void finish_data() {
	if (data_receive) {
		data_receive = false;
		*data = (char *) realloc(*data, data_len);
			
		total_size+=ATOM_SIZE+data_len;

		if (receive_dt) {
			dt_atom.type = ATOM_DT_TYPE;
			dt_atom.count = ATOM_DT_NUM;
			dt_atom.dlen = data_len+CRC_SIZE;
		} else {
			//finish atom description
			custom_atom[custom_ct].type = ATOM_CUSTOM_TYPE;
			custom_atom[custom_ct].count = 3+custom_ct;
			custom_atom[custom_ct].dlen = data_len+CRC_SIZE;
			
			custom_ct++;
		}
	}
}


int parse_command(char* cmd, char* c) {
	int val;
	uint32_t high1, high2;
	char *fn, *pull;
	char pin;
	bool valid;
	bool continue_data=false;
	
	/* Vendor info related part */
	if (strcmp(cmd, "product_uuid")==0) {
		product_serial_set = true; //required field
		high1 = 0; high2 = 0;
		
		sscanf(c, "%100s %08x-%04x-%04x-%04x-%04x%08x\n", cmd,
		       &vinf->serial[3],
		       &high1, &vinf->serial[2],
		       &high2, &vinf->serial[1],
		       &vinf->serial[0]);
		
		vinf->serial[2] |= high1<<16;
		vinf->serial[1] |= high2<<16;
		
		if ((vinf->serial[3]==0) && (vinf->serial[2]==0) &&
		    (vinf->serial[1]==0) && (vinf->serial[0]==0)) {
			//read 128 random bits from /dev/urandom
			int random_file = open("/dev/urandom", O_RDONLY);
			ssize_t result = read(random_file, vinf->serial, 16);
			close(random_file);
			if (result <= 0) {
				printf("Unable to read from /dev/urandom to set up UUID");
				return -1;
			}
			else {
				//put in the version
				vinf->serial[2] = (vinf->serial[2] & 0xffff0fff) | 0x00004000;
				
				//put in the variant
				vinf->serial[1] = (vinf->serial[1] & 0x3fffffff) | 0x80000000;
				
				printf("UUID=%08x-%04x-%04x-%04x-%04x%08x\n",
				       vinf->serial[3],
				       vinf->serial[2]>>16, vinf->serial[2] & 0xffff,
				       vinf->serial[1]>>16, vinf->serial[1] & 0xffff,
				       vinf->serial[0]);
			}
 
		}
		
	} else if (strcmp(cmd, "product_id")==0) {
		product_id_set = true; //required field
		sscanf(c, "%100s %hx", cmd, &vinf->pid);
		
	} else if (strcmp(cmd, "product_ver")==0) {
		product_ver_set = true; //required field
		sscanf(c, "%100s %hx", cmd, &vinf->pver);
		
	} else if (strcmp(cmd, "vendor")==0) {
		vendor_set = true; //required field
		
		vinf->vstr = (char*) malloc (256);
		sscanf(c, "%100s \"%255[^\"]\"", cmd, vinf->vstr);
		
		total_size-=vinf->vslen;
		vinf_atom.dlen-=vinf->vslen;
		
		vinf->vslen = strlen(vinf->vstr);
		
		total_size+=vinf->vslen;
		vinf_atom.dlen+=vinf->vslen;
		
	} else if (strcmp(cmd, "product")==0) {
		product_set = true; //required field
		
		vinf->pstr = (char*) malloc (256);
		sscanf(c, "%100s \"%255[^\"]\"", cmd, vinf->pstr);
		
		total_size-=vinf->pslen;
		vinf_atom.dlen-=vinf->pslen;
		
		vinf->pslen = strlen(vinf->pstr);
		
		total_size+=vinf->pslen;
		vinf_atom.dlen+=vinf->pslen;
	} 
	
	/* GPIO map related part */
	else if (strcmp(cmd, "gpio_drive")==0) {
		gpio_drive_set = true; //required field
		
		sscanf(c, "%100s %1x", cmd, &val);
		if (val>8 || val<0) printf("Warning: gpio_drive property in invalid region, using default value instead\n");
		else ((struct gpio_map_d *) gpio_atom.data)->flags |= val;
		
		
	} else if (strcmp(cmd, "gpio_slew")==0) {
		gpio_slew_set = true; //required field
		
		sscanf(c, "%100s %1x", cmd, &val);
		
		if (val>2 || val<0) printf("Warning: gpio_slew property in invalid region, using default value instead\n");
		else ((struct gpio_map_d *) gpio_atom.data)->flags |= val<<4;
		
	} else if (strcmp(cmd, "gpio_hysteresis")==0) {
		gpio_hysteresis_set = true; //required field
		
		sscanf(c, "%100s %1x", cmd, &val);
		
		if (val>2 || val<0) printf("Warning: gpio_hysteresis property in invalid region, using default value instead\n");
		else ((struct gpio_map_d *) gpio_atom.data)->flags |= val<<6;
		
	} else if (strcmp(cmd, "back_power")==0) {
		gpio_power_set = true; //required field
		
		sscanf(c, "%100s %1x", cmd, &val);
		
		if (val>2 || val<0) printf("Warning: back_power property in invalid region, using default value instead\n");
		else ((struct gpio_map_d *) gpio_atom.data)->power = val;
		
	} else if (strcmp(cmd, "bank1_gpio_drive")==0) {
		bank1_gpio_drive_set = true; //required field if bank 1 is used
		has_bank1 = true;

		sscanf(c, "%100s %1x", cmd, &val);

		if (val>8 || val<0) printf("Warning: bank1 gpio_drive property in invalid region, using default value instead\n");
		else ((struct gpio_map_d *) gpio_atom_bank1.data)->flags |= val;

	} else if (strcmp(cmd, "bank1_gpio_slew")==0) {
		bank1_gpio_slew_set = true; //required field if bank 1 is used
		has_bank1 = true;

		sscanf(c, "%100s %1x", cmd, &val);

		if (val>2 || val<0) printf("Warning: bank1 gpio_slew property in invalid region, using default value instead\n");
		else ((struct gpio_map_d *) gpio_atom_bank1.data)->flags |= val<<4;

	} else if (strcmp(cmd, "bank1_gpio_hysteresis")==0) {
		bank1_gpio_hysteresis_set = true; //required field if bank 1 is used
		has_bank1 = true;

		sscanf(c, "%100s %1x", cmd, &val);

		if (val>2 || val<0) printf("Warning: bank1 gpio_hysteresis property in invalid region, using default value instead\n");
		else ((struct gpio_map_d *) gpio_atom_bank1.data)->flags |= val<<6;

	} else if (strcmp(cmd, "setgpio")==0) {
		fn = (char*) malloc (101);
		pull = (char*) malloc (101);
		
		sscanf(c, "%100s %d %100s %100s", cmd, &val, fn, pull);
		
		if (val<GPIO_MIN || val>=GPIO_COUNT_TOTAL) printf("Error: GPIO number out of bounds\n");
		else {
			struct gpio_map_d *gpiomap = (struct gpio_map_d *) gpio_atom.data;

			if (val >= GPIO_COUNT) {
				gpiomap = (struct gpio_map_d *) gpio_atom_bank1.data;
				val -= GPIO_COUNT;
				has_bank1 = true;
			}

			valid = true;
			pin = 0;
			
			if (strcmp(fn, "INPUT")==0) {
				//no action
			} else if (strcmp(fn, "OUTPUT")==0) {
				pin |= 1;
			} else if (strcmp(fn, "ALT0")==0) {
				pin |= 4;
			} else if (strcmp(fn, "ALT1")==0) {
				pin |= 5;
			} else if (strcmp(fn, "ALT2")==0) {
				pin |= 6;
			} else if (strcmp(fn, "ALT3")==0) {
				pin |= 7;
			} else if (strcmp(fn, "ALT4")==0) {
				pin |= 3;
			} else if (strcmp(fn, "ALT5")==0) {
				pin |= 2;
			} else {
				printf("Error at setgpio: function type not recognised\n");
				valid=false;
			}
			
			if (strcmp(pull, "DEFAULT")==0) {
				//no action
			} else if (strcmp(pull, "UP")==0) {
				pin |= 1<<5;
			} else if (strcmp(pull, "DOWN")==0) {
				pin |= 2<<5;
			} else if (strcmp(pull, "NONE")==0) {
				pin |= 3<<5;
			} else {
				printf("Error at setgpio: pull type not recognised\n");
				valid=false;
			}
			
			pin |= 1<<7; //board uses this pin
			
			if (valid) gpiomap->pins[val] = pin;
		}
	} 
	
	/* DT atom related part */
	else if (strcmp(cmd, "dt_blob")==0) {
		finish_data();
		
		has_dt = true;
		c+=strlen("dt_blob");
		
		receive_dt=true;
		data_receive=true;
		
		data_len = 0;
		data_cap = 4;
		data = &dt_atom.data;
		*data = (char *) malloc(data_cap);
		
		if (parse_data(c)) {
			return -1;
		}
		continue_data = true;
	
	} 
	
	/* Custom data related part */
	else if (strcmp(cmd, "custom_data")==0) {
		finish_data();
		
		c+=strlen("custom_data");
		
		if (custom_cap == custom_ct) {
			custom_cap *= 2;
			custom_atom = (struct atom_t*) realloc(custom_atom, custom_cap * sizeof(struct atom_t));
		}
		
		receive_dt=false;
		data_receive=true;
		
		data_len = 0;
		data_cap = 4;
		data = &custom_atom[custom_ct].data;
		*data = (char *) malloc(data_cap);
		
		if (parse_data(c)) {
			return -1;
		}
		continue_data = true;
	
	} else if (strcmp(cmd, "end") ==0) {
		//close last data atom
		continue_data=false;
	}
	/* Incoming data */
	else if (data_receive) {
		if (parse_data(c)) {
			return -1;
		}
		continue_data = true;
	} 
	
	
	if (!continue_data) finish_data();

	return 0;
}

int read_text(char* in) {
	struct gpio_map_d* gpiomap;
	FILE * fp;
	char * line = NULL;
	char * c = NULL;
	size_t len = 0;
	ssize_t read;
	int linect = 0;
	char * command = (char*) malloc (101);
	int i;
	
	has_dt = false;
	
	printf("Opening file %s for read\n", in);
	
	fp = fopen(in, "r");
	if (fp == NULL) {
		printf("Error opening input file\n");
		return -1;
	}

	//allocating memory and setting up required atoms
	custom_cap = 1;
	custom_atom = (struct atom_t*) malloc(sizeof(struct atom_t) * custom_cap);
	
	total_size=ATOM_SIZE*2+HEADER_SIZE+VENDOR_SIZE+GPIO_SIZE;
	
	vinf_atom.type = ATOM_VENDOR_TYPE;
	vinf_atom.count = ATOM_VENDOR_NUM;
	vinf = (struct vendor_info_d *) calloc(1, sizeof(struct vendor_info_d));
	vinf_atom.data = (char *)vinf;
	vinf_atom.dlen = VENDOR_SIZE + CRC_SIZE;
	
	gpio_atom.type = ATOM_GPIO_TYPE;
	gpio_atom.count = ATOM_GPIO_NUM;
	gpiomap = (struct gpio_map_d *) calloc(1, sizeof(struct gpio_map_d));
	gpio_atom.data = (char *)gpiomap;
	gpio_atom.dlen = GPIO_SIZE + CRC_SIZE;

	gpio_atom_bank1.type = ATOM_GPIO_BANK1_TYPE;
	gpiomap = (struct gpio_map_d *) calloc(1, sizeof(struct gpio_map_d));
	gpio_atom_bank1.data = (char *)gpiomap;
	gpio_atom_bank1.dlen = GPIO_BANK1_SIZE + CRC_SIZE;
	
	while ((read = getline(&line, &len, fp)) != -1) {
		linect++;
		c = line;
		
		for (i=0; i<read; i++) if (c[i]=='#') c[i]='\0';
		
		while (isspace(*c)) ++c;
		
		
		if (*c=='\0' || *c=='\n' || *c=='\r') {
			//empty line, do nothing
		} else if (isalnum (*c)) {
			sscanf(c, "%100s", command);
			
#ifdef DEBUG
			printf("Processing line %u: %s", linect, c);
			if ((*(c+strlen(c)-1))!='\n') printf("\n");
#endif
			
			if (parse_command(command, c)) {
				return -1;
			}
		
		
		} else printf("Can't parse line %u: %s", linect, c);
	}
	
	finish_data();
	
	if (!product_serial_set || !product_id_set || !product_ver_set || !vendor_set || !product_set || 
			!gpio_drive_set || !gpio_slew_set || !gpio_hysteresis_set || !gpio_power_set) {
			
		printf("Warning: required fields missing in vendor information or GPIO map, using default values\n");
	}

	if (has_bank1 && (!bank1_gpio_drive_set || !bank1_gpio_slew_set || !bank1_gpio_hysteresis_set)) {
		printf("Warning: required fields missing in GPIO map of bank 1, using default values\n");
	}
	
	printf("Done reading\n");
	
	return 0;
}


int read_dt(char* in) {
	FILE * fp;
	unsigned long size = 0;
	
	printf("Opening DT file %s for read\n", in);
	
	fp = fopen(in, "r");
	if (fp == NULL) {
		printf("Error opening input file\n");
		return -1;
	}
	
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	printf("Adding %lu bytes of DT data\n", size);
	
	total_size+=ATOM_SIZE+size;
	has_dt = true;
	dt_atom.type = ATOM_DT_TYPE;
	dt_atom.count = ATOM_DT_NUM;
	dt_atom.dlen = size+CRC_SIZE;
	
	dt_atom.data = (char *) malloc(size);
	if (!fread(dt_atom.data, size, 1, fp)) goto err;
	
	
	fclose(fp);
	return 0;
	
err:
	printf("Unexpected EOF or error occurred\n");
	fclose(fp);
	return -1;
	
}

int read_custom(char* in) {
	FILE * fp;
	unsigned long size = 0;
	
	printf("Opening custom data file %s for read\n", in);
	
	fp = fopen(in, "r");
	if (fp == NULL) {
		printf("Error opening input file\n");
		return -1;
	}
	
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	
	printf("Adding %lu bytes of custom data\n", size);
	
	total_size+=ATOM_SIZE+size;
	
	custom_atom[custom_ct].type = ATOM_CUSTOM_TYPE;
	custom_atom[custom_ct].count = 3+custom_ct;
	custom_atom[custom_ct].dlen = size+CRC_SIZE;
	
	custom_atom[custom_ct].data = (char *) malloc(size);
	if (!fread(custom_atom[custom_ct].data, size, 1, fp)) goto err;
	
	custom_ct++;
	
	fclose(fp);
	return 0;
	
err:
	printf("Unexpected EOF or error occurred\n");
	fclose(fp);
	return -1;
	
}
int main(int argc, char *argv[]) {
	int ret;
	int i, custom_o=0;
	
	if (argc<3) {
		printf("Wrong input format.\n");
		printf("Try 'eepmake input_file output_file [dt_file] [-c custom_file_1 ... custom_file_n]'\n");
		return 1;
	}
	
	
	ret = read_text(argv[1]);
	if (ret) {
		printf("Error reading and parsing input, aborting\n");
		return 1;
	}
	
	if (argc>3) {
		if (strcmp(argv[3], "-c")==0) {
			custom_o=4;
		} else {
			//DT file specified
			if (dt_atom.dlen) total_size-=(ATOM_SIZE +dt_atom.dlen - CRC_SIZE);
			ret = read_dt(argv[3]);
			if (ret) {
				printf("Error reading DT file, aborting\n");
				return 1;
			}
		}
	}
	
	if (argc>4 && strcmp(argv[4], "-c")==0) custom_o = 5;

	if (custom_o)
		for (i = custom_o; i<argc; i++) {
			if (custom_cap == custom_ct) {
				custom_cap *= 2;
				custom_atom = (struct atom_t *)realloc(
					custom_atom, custom_cap * sizeof(struct atom_t));
			}
			//new custom data file
			ret = read_custom(argv[i]);
			if (ret) {
				printf("Error reading DT file, aborting\n");
				return 1;
			}
		}

	if (has_bank1)
		total_size += (ATOM_SIZE + GPIO_BANK1_SIZE);
	
	header.signature = htole32(HEADER_SIGN);
	header.ver = FORMAT_VERSION;
	header.res = 0;
	header.numatoms = 2+has_dt+custom_ct+has_bank1;
	header.eeplen = total_size;
	
	printf("Writing out...\n");
	
	ret = write_binary(argv[2]);
	if (ret) {
		printf("Error writing output\n");
		return 1;
	}
	
	printf("Done.\n");

	return 0;
}
