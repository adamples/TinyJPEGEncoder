#include <stdio.h>
#include <string.h>
#include "tiny_jpeg.h"

#define IMAGE_WIDTH 16
#define IMAGE_HEIGHT 8


unsigned char image[256] = {
  52, 128, 55, 128, 61, 128, 66, 128, 70, 128, 61, 128, 64, 128, 73, 128,
  52, 128, 55, 128, 61, 128, 66, 128, 70, 128, 61, 128, 64, 128, 73, 128,
  63, 128, 59, 128, 55, 128, 90, 128, 109, 128, 85, 128, 69, 128, 72, 128,
  63, 128, 59, 128, 55, 128, 90, 128, 109, 128, 85, 128, 69, 128, 72, 128,
  62, 128, 59, 128, 68, 128, 113, 128, 144, 128, 104, 128, 66, 128, 73, 128,
  62, 128, 59, 128, 68, 128, 113, 128, 144, 128, 104, 128, 66, 128, 73, 128,
  63, 128, 58, 128, 71, 128, 122, 128, 154, 128, 106, 128, 70, 128, 69, 128,
  63, 128, 58, 128, 71, 128, 122, 128, 154, 128, 106, 128, 70, 128, 69, 128,
  67, 128, 61, 128, 68, 128, 104, 128, 126, 128, 88, 128, 68, 128, 70, 128,
  67, 128, 61, 128, 68, 128, 104, 128, 126, 128, 88, 128, 68, 128, 70, 128,
  79, 128, 65, 128, 60, 128, 70, 128, 77, 128, 68, 128, 58, 128, 75, 128,
  79, 128, 65, 128, 60, 128, 70, 128, 77, 128, 68, 128, 58, 128, 75, 128,
  85, 128, 71, 128, 64, 128, 59, 128, 55, 128, 61, 128, 65, 128, 83, 128,
  85, 128, 71, 128, 64, 128, 59, 128, 55, 128, 61, 128, 65, 128, 83, 128,
  87, 128, 79, 128, 69, 128, 68, 128, 65, 128, 76, 128, 78, 128, 94, 128,
  87, 128, 79, 128, 69, 128, 68, 128, 65, 128, 76, 128, 78, 128, 94, 128
};


int main(void)
{
  jpeg_proc_t               processor;
  const jpeg_file_header_t  *header;
  uint8_t                   output_buffer[0xffff];
  int                       file_size = 0;
  int                       bytes;

  tjpeg_init(&processor, IMAGE_WIDTH, IMAGE_HEIGHT);

  header = tjpeg_get_header();
  memcpy(output_buffer, header->data, header->length);
  output_buffer[header->image_width_offset] = (uint8_t) (IMAGE_WIDTH >> 8);
  output_buffer[header->image_width_offset + 1] = (uint8_t) (IMAGE_WIDTH & 0xff);
  output_buffer[header->image_height_offset] = (uint8_t) (IMAGE_HEIGHT >> 8);
  output_buffer[header->image_height_offset + 1] = (uint8_t) (IMAGE_HEIGHT & 0xff);
  file_size = header->length;
  printf("File size: %d.\n", file_size);

  tjpeg_init(&processor, IMAGE_WIDTH, IMAGE_HEIGHT);
  tjpeg_feed_data(&processor, IMAGE_WIDTH, IMAGE_HEIGHT, image);
  bytes = tjpeg_write(&processor, output_buffer + file_size, sizeof(output_buffer));
  file_size += bytes;
  printf("File size: %d.\n", file_size);
  printf("Header size: %d.\n", header->length);

  FILE* file = fopen("image.jpeg", "wb");
  fwrite(output_buffer, file_size, 1, file);
  fclose(file);
}
