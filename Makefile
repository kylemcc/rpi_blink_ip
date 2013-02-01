CFLAGS=-Wall

all: blink.c
	make clean
	make blink 

blink: blink.c
	gcc $(CFLAGS) $(USERDEF) blink.c -o blink

.PHONY: clean
clean:
	rm -f blink
