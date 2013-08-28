include Makefile.inc

all: library test

library:
	make all -C src

test:
	make all -C test

clean:
	make clean -C test
	make clean -C src

.PHONY: all clean library test
