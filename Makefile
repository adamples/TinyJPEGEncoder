include Makefile.inc

all clean:
	make $@ -C src -I..
	make $@ -C test -I..
