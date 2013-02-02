CC=gcc
CFLAGS=-Wall

all: blink.c
	make clean
	make blink 

blink: blink.c
	$(CC) $(CFLAGS) $(USERDEF) -DRPI -DNDEBUG blink.c -o blink

debug: blink.c
	$(CC) $(CFLAGS) $(USERDEF) blink.c -o dblink

.PHONY: clean
clean:
	rm -f blink dblink
