# Current tools are eepmake and eepdump

CC ?= gcc

all: eepmake eepdump

eepmake: eeptypes.h eepmake.c
	$(CC) eepmake.c -o eepmake -Wno-format

eepdump: eeptypes.h eepdump.c
	$(CC) eepdump.c -o eepdump -Wno-format

clean:
	rm -f eepmake eepdump
