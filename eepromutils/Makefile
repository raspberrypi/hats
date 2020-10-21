# Current tools are eepmake and eepdump

CC ?= gcc
CFLAGS ?= -Wall -Wextra

all: eepmake eepdump

eepmake: eeptypes.h eepmake.c
	$(CC) $(CFLAGS) eepmake.c -o eepmake

eepdump: eeptypes.h eepdump.c
	$(CC) $(CFLAGS) eepdump.c -o eepdump

clean:
	rm -f eepmake eepdump
