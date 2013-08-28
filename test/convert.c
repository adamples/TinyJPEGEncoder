#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "tiny_jpeg.h"


int main(int argc, char **argv)
{
  jpeg_proc_t               processor;
  const jpeg_file_header_t  *header;

  int       width, height, offset;
  uint8_t   *input_image;
  uint8_t   *ycbcr_image;
  uint8_t   output_image[0xffff];
  int       file_size = 0;
  int       bytes;

  FILE* input_file = fopen(argv[1], "rb");
  fread(output_image, 54, 1, input_file);
  width = *((int32_t *) (output_image + 18));
  height = *((int32_t *) (output_image + 22));
  offset = *((int32_t *) (output_image + 10));
  input_image = malloc(width * height * 3);
  fread(input_image, offset - 54, 1, input_file);
  fread(input_image, width * height * 3, 1, input_file);
  fclose(input_file);

  ycbcr_image = malloc(width * height * 2);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; x += 2) {
      double r1 = input_image[3 * (x + (height - y - 1) * width) + 2];
      double g1 = input_image[3 * (x + (height - y - 1) * width) + 1];
      double b1 = input_image[3 * (x + (height - y - 1) * width)];
      double r2 = input_image[3 * (x + (height - y - 1) * width) + 5];
      double g2 = input_image[3 * (x + (height - y - 1) * width) + 4];
      double b2 = input_image[3 * (x + (height - y - 1) * width) + 3];

      double cb1 = 128.0 - (0.168736 * r1) - (0.331264 * g1) + (0.5 * b1);
      double cb2 = 128.0 - (0.168736 * r2) - (0.331264 * g2) + (0.5 * b2);
      double cr1 = 128.0 + (0.5 * r1) - (0.418688 * g1) - (0.081312 * b1);
      double cr2 = 128.0 + (0.5 * r2) - (0.418688 * g2) - (0.081312 * b2);

      ycbcr_image[2 * (x + y * width)] = 0.299 * r1 + 0.587 * g1 + 0.114 * b1;
      ycbcr_image[2 * (x + y * width) + 2] = 0.299 * r2 + 0.587 * g2 + 0.114 * b2;
      ycbcr_image[2 * (x + y * width) + 3] = (cb1 + cb2) / 2;
      ycbcr_image[2 * (x + y * width) + 1] = (cr1 + cr2) / 2;
    }

    //~ printf("\n");
  }

  tjpeg_init(&processor, width, height);

  FILE* output_file = fopen(argv[2], "wb");

  header = tjpeg_get_header();
  memcpy(output_image, header->data, header->length);
  output_image[header->image_width_offset] = (uint8_t) (width >> 8);
  output_image[header->image_width_offset + 1] = (uint8_t) (width & 0xff);
  output_image[header->image_height_offset] = (uint8_t) (height >> 8);
  output_image[header->image_height_offset + 1] = (uint8_t) (height & 0xff);
  file_size = header->length;

  fwrite(output_image, header->length, 1, output_file);

  tjpeg_init(&processor, width, height);

  tjpeg_feed_data(&processor, width, height, ycbcr_image);

  for (int i = 0; i < 1; ++i) {

    do {
      bytes = tjpeg_write(&processor, output_image, 8192);
      fwrite(output_image, bytes, 1, output_file);
      fflush(output_file);
      file_size += bytes;
    } while (bytes > 0);
  }

  fclose(output_file);

  return 0;
}
