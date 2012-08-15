# Wszystkie komunikaty o błędach, C99
CFLAGS+=-Wall
CFLAGS+=-std=gnu99
CFLAGS+=-g
CFLAGS+=-O0
CFLAGS+=-DDEBUG

LDLIBS+=-lm

.PHONY: all clean

all: tiny_jpeg.o dct.o

clean:
	rm -f *.o *.c~ *.h~

