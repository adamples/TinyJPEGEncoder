#include "tiny_jpeg.h"
#include "dct.h"

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


static const int ZIGZAG[64] = {
    0,
    1,  8,
   16,  9,  2,
    3, 10, 17, 24,
   32, 25, 18, 11,  4,
    5, 12, 19, 26, 33, 40,
   48, 41, 34, 27, 20, 13,  6,
    7, 14, 21, 28, 35, 42, 49, 56,
   57, 50, 43, 36, 29, 22, 15,
   23, 30, 37, 44, 51, 58,
   59, 52, 45, 38, 31,
   39, 46, 53, 60,
   61, 54, 47,
   55, 62,
   63
};

static const uint16_t CODE_TO_HUF_Y_DC[12] = {
  0x0063, 0x0053, 0x0033, 0x0023, 0x0003, 0x0013, 0x0043,
  0x00e4, 0x01e5, 0x03e6, 0x07e7, 0x0fe8
};

/**
 * Table is indexed by [run][length]
 * Lowest byte is code length.
 */
static const uint16_t CODE_TO_HUF_Y_AC[] = {
  0x00c3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
  0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xff3b,
  0x0001, 0x00a3, 0x01b4, 0x03a5, 0x03b5, 0x0786, 0x07b6, 0x0f97,
  0x1f48, 0x1f78, 0x1f88, 0x3f59, 0x3f89, 0x3fa9, 0x7f6a, 0xfedc,
  0x0011, 0x01c4, 0x0f87, 0x1f68, 0x3f69, 0x3f99, 0xff2b, 0xff5b,
  0xfecc, 0xfe0d, 0xfe1d, 0xfc4e, 0xf8df, 0xf9af, 0xf9bf, 0xfaef,
  0x0042, 0x07a6, 0x3f79, 0x7f7a, 0xfddd, 0xfdfd, 0xfc5e, 0xf8cf,
  0xf9ef, 0xf91f, 0xf92f, 0xf93f, 0xf99f, 0xfaaf, 0xfaff, 0xfb0f,
  0x00b3, 0x1f58, 0xff4b, 0xfded, 0xf8ff, 0xf96f, 0xf97f, 0xf98f,
  0xfa3f, 0xfa4f, 0xfa7f, 0xfa8f, 0xfb1f, 0xfb2f, 0xfb3f, 0xfb4f,
  0x01a4, 0x3f49, 0xfdcd, 0xf8ef, 0xfa5f, 0xfabf, 0xfb5f, 0xfb6f,
  0xfb7f, 0xfb8f, 0xfb9f, 0xfbaf, 0xfbbf, 0xfbcf, 0xfbdf, 0xfbef,
  0x0796, 0x7f8a, 0xf9df, 0xf94f, 0xfa6f, 0xfa9f, 0xfbff, 0xfc0f,
  0xfc1f, 0xfc2f, 0xfc3f, 0xfc4f, 0xfc5f, 0xfc6f, 0xfc7f, 0xfc8f,
  0x1f98, 0xf95f, 0xf90f, 0xfc9f, 0xfcaf, 0xfcbf, 0xfccf, 0xfcdf,
  0xfcef, 0xfcff, 0xfd0f, 0xfd1f, 0xfd2f, 0xfd3f, 0xfd4f, 0xfd5f,
  0xf9cf, 0xfa1f, 0xfacf, 0xfd6f, 0xfd7f, 0xfd8f, 0xfd9f, 0xfdaf,
  0xfdbf, 0xfdcf, 0xfddf, 0xfdef, 0xfdff, 0xfe0f, 0xfe1f, 0xfe2f,
  0xf9ff, 0xfa2f, 0xfe3f, 0xfe4f, 0xfe5f, 0xfe6f, 0xfe7f, 0xfe8f,
  0xfe9f, 0xfeaf, 0xfebf, 0xfecf, 0xfedf, 0xfeef, 0xfeff, 0xff0f,
  0xfa0f, 0xfadf, 0xff1f, 0xff2f, 0xff3f, 0xff4f, 0xff5f, 0xff6f,
  0xff7f, 0xff8f, 0xff9f, 0xffaf, 0xffbf, 0xffcf, 0xffdf, 0xffef
};

static const uint16_t CODE_TO_HUF_C_DC[12] = {
  0x0012, 0x0002,
  0x0043, 0x0053,
  0x00c4, 0x00d4, 0x00e4,
  0x01e5, 0x03e6, 0x07e7, 0x0fe8, 0x01fe9
};


/**
 * JFIF file header
 */
static const uint8_t FILE_HEADER_DATA[] = {
  0xff, MARKER_SOI,
    0xff, MARKER_APP0,              /* Application header */
      0x00, 0x10,                   /* Segment length (16 bytes) */
      0x4a, 0x46, 0x49, 0x46, 0x00, /* "JFIF\0" */
      0x01, 0x00,                   /* v1.0 */
      0x00,                         /* Density unit: none */
      0x00, 0x01,                   /* Aspect ratio: 1:1 (X part) */
      0x00, 0x01,                   /* Aspect ratio: 1:1 (Y part) */
      0x00, 0x00,                   /* Thumbnail size (no thumbnail) */
    0xff, MARKER_DQT,               /* Quantization table */
      0x00, 0x43,                   /* Segment length (67 bytes) */
      0x00,                         /* Quantization matrix: index 0, precision 8 bits */
      0x10, 0x0b, 0x0c, 0x0e, 0x0c, 0x0a, 0x10, 0x0e,
      0x0d, 0x0e, 0x12, 0x11, 0x10, 0x13, 0x18, 0x28,
      0x1a, 0x18, 0x16, 0x16, 0x18, 0x31, 0x23, 0x25,
      0x1d, 0x28, 0x3a, 0x33, 0x3d, 0x3c, 0x39, 0x33,
      0x38, 0x37, 0x40, 0x48, 0x5c, 0x4e, 0x40, 0x44,
      0x57, 0x45, 0x37, 0x38, 0x50, 0x6d, 0x51, 0x57,
      0x5f, 0x62, 0x67, 0x68, 0x67, 0x3e, 0x4d, 0x71,
      0x79, 0x70, 0x64, 0x78, 0x5c, 0x65, 0x67, 0x63,
    0xff, MARKER_SOF0,              /* Start of frame */
      0x00, 0x11,                   /* Segment length (17 bytes) */
      0x08,                         /* Precision/bpp (8bpp) */
      0x00, 0x00,                   /* Image height (replace) */
      0x00, 0x00,                   /* Image width (replace) */
      0x03,                         /* Number of componends (3/YCbCr) */
      0x01, 0x21, 0x00,   /* Y component, sampling 1/1, quantization table 0 */
      0x02, 0x11, 0x00,   /* Cb component, sampling 2/1, quantization table 0 */
      0x03, 0x11, 0x00,   /* Cr component, sampling 2/1, quantization table 0 */
    0xff, MARKER_DHT,               /* Huffman table */
      0x00, 0x1f,                   /* Segment size (31 bytes) */
      0x00,                         /* Table Y DC: index 0 */
      0x00, 0x00, 0x07, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      /* Symbols */
      0x04, 0x05, 0x03, 0x02, 0x06, 0x01, 0x00,
      0x07, 0x08, 0x09, 0x0a, 0x0b,
/*
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0a, 0x0b,*/
    0xff, MARKER_DHT,
      0x00, 0xb5,                   /* Segment size (181 bytes) */
      0x10,                         /* Table Y AC: index 1 */
      0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x02,
      0x06, 0x07, 0x03, 0x04, 0x02, 0x06, 0x02, 0x73,
      0x01, 0x02,
      0x03,
      0x11, 0x04, 0x00,
      0x05, 0x21, 0x12,
      0x31, 0x41,
      0x51, 0x06, 0x13, 0x61,
      0x22, 0x71,
      0x81, 0x14, 0x32, 0x91, 0xa1, 0x07,
      0x15, 0xb1, 0x42, 0x23, 0xc1, 0x52, 0xd1,
      0xe1, 0x33, 0x16,
      0x62, 0xf0, 0x24, 0x72,
      0x82, 0xf1,
      0x25, 0x43, 0x34, 0x53, 0x92, 0xa2,
      0xb2, 0x63,
      0x73, 0xc2, 0x35, 0x44, 0x27, 0x93, 0xa3, 0xb3,
      0x36, 0x17, 0x54, 0x64, 0x74, 0xc3, 0xd2, 0xe2,
      0x08, 0x26, 0x83, 0x09, 0x0a, 0x18, 0x19, 0x84,
      0x94, 0x45, 0x46, 0xa4, 0xb4, 0x56, 0xd3, 0x55,
      0x28, 0x1a, 0xf2, 0xe3, 0xf3, 0xc4, 0xd4, 0xe4,
      0xf4, 0x65, 0x75, 0x85, 0x95, 0xa5, 0xb5, 0xc5,
      0xd5, 0xe5, 0xf5, 0x66, 0x76, 0x86, 0x96, 0xa6,
      0xb6, 0xc6, 0xd6, 0xe6, 0xf6, 0x37, 0x47, 0x57,
      0x67, 0x77, 0x87, 0x97, 0xa7, 0xb7, 0xc7, 0xd7,
      0xe7, 0xf7, 0x38, 0x48, 0x58, 0x68, 0x78, 0x88,
      0x98, 0xa8, 0xb8, 0xc8, 0xd8, 0xe8, 0xf8, 0x29,
      0x39, 0x49, 0x59, 0x69, 0x79, 0x89, 0x99, 0xa9,
      0xb9, 0xc9, 0xd9, 0xe9, 0xf9, 0x2a, 0x3a, 0x4a,
      0x5a, 0x6a, 0x7a, 0x8a, 0x9a, 0xaa, 0xba, 0xca,
      0xda, 0xea, 0xfa,
    0xff, MARKER_SOS,               /* Start of scan */
      0x00, 0x0c,                   /* Segment length (12 bytes) */
      0x03,                         /* 3 components */
      0x01, 0x00,                   /* Y component,  DC table: 0, AC table: 1 */
      0x02, 0x00,                   /* Cb component, DC table: 0, AC table: 1 */
      0x03, 0x00,                   /* Cr component, DC table: 0, AC table: 1 */
      0x00, 0x3f, 0x00              /* 3 bytes to ignore */
};

#define HEADER_IMAGE_HEIGHT_OFFSET (94)
#define HEADER_IMAGE_WIDTH_OFFSET  (96)

static const jpeg_file_header_t FILE_HEADER = {
  .length = sizeof(FILE_HEADER_DATA),
  .data = FILE_HEADER_DATA,
  .image_width_offset = HEADER_IMAGE_WIDTH_OFFSET,
  .image_height_offset = HEADER_IMAGE_HEIGHT_OFFSET
};

static const int8_t QUANTIZATION_MATRIX[64] = {
  16,  11,  10,  16,  24,  40,  51,  61,
  12,  12,  14,  19,  26,  58,  60,  55,
  14,  13,  16,  24,  40,  57,  69,  56,
  14,  17,  22,  29,  51,  87,  80,  62,
  18,  22,  37,  56,  68, 109, 103,  77,
  24,  35,  55,  64,  81, 104, 113,  92,
  49,  64,  78,  87, 103, 121, 120, 101,
  72,  92,  95,  98, 112, 100, 103,  99
};


void tjpeg_quantize(int16_t *block)
{
  int8_t *qm = (int8_t *) QUANTIZATION_MATRIX;

  for (int i = 0; i < 64; ++i, ++block, ++qm)
  {
    // Tutaj można dodać zaokrąglanie, ale to bardzo zwalnia
    assert(*qm != 0);
    assert(*block > -2048 && *block < 2048);
    assert(false);

    *block = 2 * *block / *qm;

    //~ if (*block % 2 == 0)
      //~ *block /= 2;
    //~ else
      //~ if (*block > 0)
        //~ *block = *block / 2 + 1;
      //~ else
        //~ *block = *block / 2 - 1;

    switch (*block & 0x80000001) {
      case 0x80000001 : *block = *block / 2 - 1; break;
      case 0x00000001 : *block = *block / 2 + 1; break;
      default : *block /= 2;
    }
    //*block /= *qm;
  }
}


void tjpeg_encode(jpeg_proc_t *p, int16_t *last_dc, const uint16_t *dc_table, const uint16_t *ac_table, int16_t *block)
{
  tjpeg_buffer_add_dc(&p->buffer, dc_table, block[0] - *last_dc);
  *last_dc = block[0];

  for (uint8_t i = 1; i < 64; ++i) {
    uint8_t run = 0;

    while (i < 64 && block[ZIGZAG[i]] == 0) {
      ++i;
      ++run;
    }

    if (i == 64) {
      tjpeg_buffer_add_ac(&p->buffer, ac_table, 0, 0);
      break;
    }
    else
    {
      while (run >= 16) {
        tjpeg_buffer_add_ac(&p->buffer, ac_table, 15, 0);
        run -= 16;
      }
    }

    tjpeg_buffer_add_ac(&p->buffer, CODE_TO_HUF_Y_AC, run, block[ZIGZAG[i]]);
  }
}


static inline void tjpeg_compress_block(int16_t *block)
{
  //reference_dct(block);
  fastest_dct(block);
  tjpeg_quantize(block);
}


const jpeg_file_header_t *tjpeg_get_header(void)
{
  return &FILE_HEADER;
}


void tjpeg_init(jpeg_proc_t *p, uint16_t width, uint16_t height)
{
  tjpeg_buffer_init(&p->buffer);
  p->image_width = width;
  p->image_height = height;
  p->last_y1_dc = 0;
  p->last_y2_dc = 0;
  p->last_cr_dc = 0;
  p->last_cb_dc = 0;
  p->x_pos = 0;
  p->y_pos = 0;
  p->eoi = false;
  p->blocks_n = 0;
}


void tjpeg_feed_data(jpeg_proc_t *p, uint16_t width, uint16_t height, uint8_t *data)
{
  tjpeg_image_chunk_init(&p->chunk, data, width, height);
}


int tjpeg_write(jpeg_proc_t *p, uint8_t* destination, int size)
{
  int     bytes_written = 0;
  int16_t block[64];

  while (bytes_written < size) {
    /* Check if there is data available in internal buffer */
    if (tjpeg_buffer_get_length(&p->buffer) > 0) {
      int to_copy;

      if (tjpeg_buffer_get_length(&p->buffer) >= size - bytes_written)
        to_copy = size - bytes_written;
      else
        to_copy = tjpeg_buffer_get_length(&p->buffer);

      tjpeg_buffer_copy(&p->buffer, destination, to_copy);
      destination += to_copy;
      bytes_written += to_copy;
    }
    /* Produce more data */
    else
    {
      if (tjpeg_image_chunk_next_block(&p->chunk)) {
        tjpeg_image_chunk_copy_y1(&p->chunk, block);
        tjpeg_compress_block(block);
        tjpeg_encode(p, &p->last_y1_dc, CODE_TO_HUF_Y_DC, CODE_TO_HUF_Y_AC, block);

        tjpeg_image_chunk_copy_y2(&p->chunk, block);
        tjpeg_compress_block(block);
        tjpeg_encode(p, &p->last_y1_dc, CODE_TO_HUF_Y_DC, CODE_TO_HUF_Y_AC, block);

        tjpeg_image_chunk_copy_cr(&p->chunk, block);
        tjpeg_compress_block(block);
        tjpeg_encode(p, &p->last_cr_dc, CODE_TO_HUF_Y_DC, CODE_TO_HUF_Y_AC, block);

        tjpeg_image_chunk_copy_cb(&p->chunk, block);
        tjpeg_compress_block(block);
        tjpeg_encode(p, &p->last_cb_dc, CODE_TO_HUF_Y_DC, CODE_TO_HUF_Y_AC, block);

        p->blocks_n += 1;
      } else {
        p->x_pos += p->chunk.width;

        if (p->x_pos >= p->image_width) {
          p->x_pos = 0;
          p->y_pos += p->chunk.height;
        }

        if (p->y_pos >= p->image_height) {
          if (p->eoi) {
            break;
          }
          else {
            tjpeg_buffer_add_byte(&p->buffer, 0xff);
            tjpeg_buffer_add_byte(&p->buffer, MARKER_EOI);
            p->eoi = true;
          }
        }
        else {
          break;
        }
      }
    }
  }

  return bytes_written;
}
