include ../Makefile.inc

TARGET = tiny_jpeg.a
C_SRC = \
	dct.c \
	tjpeg_buffer.c \
	tjpeg_image_chunk.c \
	tiny_jpeg.c \
	$(NULL)

C_OBJ = $(C_SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(C_OBJ) $(TARGET)($(C_OBJ))

tiny_jpeg.o: tjpeg_qmatrix.h

tjpeg_qmatrix.h: tiny_jpeg.h generate_qmatrix.sh
	sh generate_qmatrix.sh tiny_jpeg.h tjpeg_qmatrix.h

clean:
	rm -f -- $(TARGET)
	rm -f -- $(C_OBJ)
	rm -f -- $(C_SRC:.c=.d)
	rm -f -- tjpeg_qmatrix.h

.PHONY: \
	all clean \
	$(NULL)

-include $(C_SRC:.c=.d)
