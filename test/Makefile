CFLAGS+=-Wall
CFLAGS+=-std=gnu99
CFLAGS+=-g
CFLAGS+=-pg
CFLAGS+=-O2
CFLAGS+=-DNDEBUG
CFLAGS+=-I../src/

LDLIBS+=-lm

TARGETS=test_buffer

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f *.o *.c~ *.h~ $(TARGETS)

test_1: ../src/tiny_jpeg.o ../src/dct.o ../src/tjpeg_buffer.o ../src/tjpeg_image_chunk.o
convert: ../src/tiny_jpeg.o ../src/dct.o ../src/tjpeg_buffer.o ../src/tjpeg_image_chunk.o

test_buffer: ../src/tjpeg_buffer.o
test_image_chunk: ../src/tjpeg_image_chunk.o
test_dct: ../src/dct.o
