include ../Makefile.inc

TARGETS = \
	convert \
	test_buffer \
	test_image_chunk \
	$(NULL)
#	test_dct \
#	test_divide \

C_OBJ = $(TARGETS:=.o)
OBJLIBS = ../src/tiny_jpeg.a
CFLAGS += -I../src/

.PHONY: \
	all clean \
	lib_tiny_jpeg \
	$(NULL)

all: $(TARGETS)

$(TARGETS): $(OBJLIBS)

../src/tiny_jpeg.a: lib_tiny_jpeg

lib_tiny_jpeg:
	make tiny_jpeg.a -C ../src

clean:
	rm -f -- $(TARGETS)
	rm -f -- $(C_OBJ)
	rm -f -- $(TARGETS:=.d)

-include $(TARGET:=.d)
