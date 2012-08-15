# Wszystkie komunikaty o błędach, C99
CFLAGS+=-Wall
CFLAGS+=-std=gnu99
CFLAGS+=-g
CFLAGS+=-O0
CFLAGS+=-I../src/
LDLIBS+=-lm


TARGETS=test_1

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f *.o *.c~ *.h~ $(TARGETS)

$(TARGETS): ../src/tiny_jpeg.o ../src/dct.o
