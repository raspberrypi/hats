# Current tools are eepmake and eepdump

all: eepmake eepdump
	
eepmake: eeptypes.h eepmake.c
	gcc eepmake.c -o eepmake -Wno-format

eepdump: eeptypes.h eepdump.c
	gcc eepdump.c -o eepdump -Wno-format
	
clean:
	rm -f eepmake eepdump