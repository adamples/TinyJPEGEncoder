#include "tiny_jpeg.h"
#include "dct.h"

#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include <stdio.h>


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

static const int16_t CODE_TO_HUF_Y_DC[12] = {
  0x0063, 0x0053, 0x0033, 0x0023, 0x0003, 0x0013, 0x0043,
  0x00e4, 0x01e5, 0x03e6, 0x07e7, 0x0fe8
};

/**
 * Table is indexed by [run][length]
 * Lowest byte is code length.
 */
static const int16_t CODE_TO_HUF_Y_AC[16][10] = {
  { 0x00a4, 0x0002, 0x0012, 0x0043, 0x00b4, 0x01a5, 0x0787, 0x0f88, 0x3f6a, 0xff82 },
  { 0x0000, 0x00c4, 0x01b5, 0x0797, 0x1f69, 0x7f6b, 0xff84, 0xff85, 0xff86, 0xff87 },
  { 0x0000, 0x01c5, 0x0f98, 0x3f7a, 0xff4c, 0xff89, 0xff8a, 0xff8b, 0xff8c, 0xff8d },
  { 0x0000, 0x03a6, 0x1f79, 0xff5c, 0xff8f, 0xff90, 0xff91, 0xff92, 0xff93, 0xff94 },
  { 0x0000, 0x03b6, 0x3f8a, 0xff96, 0xff97, 0xff98, 0xff99, 0xff9a, 0xff9b, 0xff9c },
  { 0x0000, 0x07a7, 0x7f7b, 0xff9e, 0xff9f, 0xffa0, 0xffa1, 0xffa2, 0xffa3, 0xffa4 },
  { 0x0000, 0x07b7, 0xff6c, 0xffa6, 0xffa7, 0xffa8, 0xffa9, 0xffaa, 0xffab, 0xffac },
  { 0x0000, 0x0fa8, 0xff7c, 0xffae, 0xffaf, 0xffb0, 0xffb1, 0xffb2, 0xffb3, 0xffb4 },
  { 0x0000, 0x1f89, 0xff81, 0xffb6, 0xffb7, 0xffb8, 0xffb9, 0xffba, 0xffbb, 0xffbc },
  { 0x0000, 0x1f99, 0xffbe, 0xffbf, 0xffc0, 0xffc1, 0xffc2, 0xffc3, 0xffc4, 0xffc5 },
  { 0x0000, 0x1fa9, 0xffc7, 0xffc8, 0xffc9, 0xffca, 0xffcb, 0xffcc, 0xffcd, 0xffce },
  { 0x0000, 0x3f9a, 0xffd0, 0xffd1, 0xffd2, 0xffd3, 0xffd4, 0xffd5, 0xffd6, 0xffd7 },
  { 0x0000, 0x3faa, 0xffd9, 0xffda, 0xffdb, 0xffdc, 0xffdd, 0xffde, 0xffdf, 0xffe0 },
  { 0x0000, 0x7f8b, 0xffe2, 0xffe3, 0xffe4, 0xffe5, 0xffe6, 0xffe7, 0xffe8, 0xffe9 },
  { 0x0000, 0xffeb, 0xffec, 0xffed, 0xffee, 0xffef, 0xfff0, 0xfff1, 0xfff2, 0xfff3 },
  { 0x7f9b, 0xfff5, 0xfff6, 0xfff7, 0xfff8, 0xfff9, 0xfffa, 0xfffb, 0xfffc, 0xfffd }
};

static const int16_t CODE_TO_HUF_C_DC[12] = {
  0x0012, 0x0002,
  0x0043, 0x0053,
  0x00c4, 0x00d4, 0x00e4,
  0x01e5, 0x03e6, 0x07e7, 0x0fe8, 0x01fe9
};


/**
 * JFIF file header (288 bytes)
 */
static const uint8_t FILE_HEADER[288] = {
  0xff, MARKER_SOI,
    0xff, MARKER_APP0,              /* Application header */
      0x00, 0x12,                   /* Segment length (16 bytes) */
      0x4a, 0x46, 0x49, 0x46, 0x00, /* "JFIF\0" */
      0x01, 0x00,                   /* v1.0 */
      0x00,                         /* Density unit: none */
      0x00, 0x01,                   /* Aspect ratio: 1:1 (X part) */
      0x00, 0x01,                   /* Aspect ratio: 1:1 (Y part) */
      0x00, 0x00,                   /* Thumbnail size (no thumbnail) */
    0xff, MARKER_DQT,               /* Quantization table */
      0x00, 0x43,                   /* Segment length (67 bytes) */
      0x00,                         /* Quantization matrix: index 0, precision 8 bits */
      16,  11,  10,  16,  24,  40,  51,  61,
      12,  12,  14,  19,  26,  58,  60,  55,
      14,  13,  16,  24,  40,  57,  69,  56,
      14,  17,  22,  29,  51,  87,  80,  62,
      18,  22,  37,  56,  68, 109, 103,  77,
      24,  35,  55,  64,  81, 104, 113,  92,
      49,  64,  78,  87, 103, 121, 120, 101,
      72,  92,  95,  98, 112, 100, 103,  99,
    0xff, MARKER_SOF0,              /* Start of frame */
      0x00, 0x0e,                   /* Segment length (14 bytes) */
      0x08,                         /* Precision/bpp (8bpp) */
      0x00, 0x00,                   /* Image height (replace) */
      0x00, 0x00,                   /* Image width (replace) */
      0x03,                         /* Number of componends (3/YCbCr) */
      0x01, 0x11, 0x00,   /* Y component, sampling 1/1, quantization table 0 */
      0x02, 0x12, 0x00,   /* Cb component, sampling 2/1, quantization table 0 */
      0x03, 0x12, 0x00,   /* Cr component, sampling 2/1, quantization table 0 */
    0xff, MARKER_DHT,               /* Huffman table */
      0x00, 0xbb,                   /* Segment size (167 bytes) */
      0x00,                         /* Table Y DC: index 0 */
      0x00, 0x00, 0x07, 0x01, 0x01, 0x01, 0x01, 0x01,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      /* Symbols */
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0a, 0x0b,
      0x13,                         /* Table Y AC: index 1 */
      0x07, 0x0a, 0x08, 0x0a, 0x08, 0x09, 0x0c, 0x09,
      0x0d, 0x0c, 0x0c, 0x0c, 0x05, 0x06, 0x06, 0x00,
      /* Symbols */
      0x36, 0x56, 0x76, 0x82, 0x95, 0xb3, 0xe7,
      0x01, 0x02, 0x09, 0x37, 0x57, 0x77, 0x96, 0xb4, 0xd2, 0xe8,
      0x03, 0x38, 0x58, 0x78, 0x97, 0xb5, 0xd3, 0xe9,
      0x00, 0x04, 0x11, 0x16, 0x39, 0x59, 0x79, 0x98, 0xb6, 0xd4,
      0x05, 0x12, 0x17, 0x21, 0x99, 0xb7, 0xd5, 0xf1,
      0x18, 0x31, 0x41, 0x43, 0x63, 0x83, 0xb8, 0xd6, 0xf2,
      0x06, 0x13, 0x19, 0x44, 0x51, 0x61, 0x64, 0x84, 0xa2, 0xb9, 0xd7, 0xf3,
      0x07, 0x22, 0x45, 0x65, 0x71, 0x85, 0xa3, 0xd8, 0xf4,
      0x14, 0x25, 0x32, 0x46, 0x66, 0x81, 0x86, 0x91, 0xa1, 0xa4, 0xc2, 0xd9, 0xf5,
      0x08, 0x23, 0x26, 0x42, 0x47, 0x67, 0x87, 0xa5, 0xb1, 0xc1, 0xc3, 0xf6,
      0x15, 0x27, 0x48, 0x52, 0x68, 0x88, 0xa6, 0xc4, 0xd1, 0xe1, 0xf0, 0xf7,
      0x24, 0x28, 0x33, 0x49, 0x62, 0x69, 0x72, 0x89, 0xa7, 0xc5, 0xe2, 0xf8,
      0x29, 0xa8, 0xc6, 0xe3, 0xf9,
      0x53, 0x73, 0x92, 0xa9, 0xc7, 0xe4,
      0x34, 0x54, 0x74, 0x93, 0xc8, 0xe5,
    0xff, MARKER_SOS,               /* Start of scan */
      0x00, 0x0c,                   /* Segment length (12 bytes) */
      0x03,                         /* 3 components */
      0x01, 0x01,                   /* Y component,  DC table: 0, AC table: 1 */
      0x02, 0x01,                   /* Cb component, DC table: 0, AC table: 1 */
      0x03, 0x01,                   /* Cr component, DC table: 0, AC table: 1 */
      0x00, 0x00, 0x00              /* 3 bytes to ignore */
};


/*static const int8_t QUANTIZATION_MATRIX[64] = {
  16,  11,  10,  16,  24,  40,  51,  61,
  12,  12,  14,  19,  26,  58,  60,  55,
  14,  13,  16,  24,  40,  57,  69,  56,
  14,  17,  22,  29,  51,  87,  80,  62,
  18,  22,  37,  56,  68, 109, 103,  77,
  24,  35,  55,  64,  81, 104, 113,  92,
  49,  64,  78,  87, 103, 121, 120, 101,
  72,  92,  95,  98, 112, 100, 103,  99
};*/

static const uint16_t HEADER_IMAGE_HEIGHT_OFFSET = 94;
static const uint16_t HEADER_IMAGE_WIDTH_OFFSET = 96;

static const int8_t *QUANTIZATION_MATRIX = (int8_t *) (FILE_HEADER + 25);


void print_int16(int16_t *data) {
  printf("[\n");

  for (int y = 0; y < 8; ++y) {
    printf("  ");
    for (int x = 0; x < 8; ++x) {
      printf("%3d", data[x + y * 8]);
      if (x != 7) printf(", ");
    }
    printf(";\n");
  }

  printf("]\n");
}


void print_int8(int8_t *data) {
  printf("[\n");

  for (int y = 0; y < 8; ++y) {
    printf("  ");
    for (int x = 0; x < 8; ++x) {
      printf("%3d", data[x + y * 8]);
      if (x != 7) printf(", ");
    }
    printf(";\n");
  }

  printf("]\n");
}


void tjpeg_quantize(int16_t *block)
{
  for (uint8_t i = 0; i < 64; ++i) {
    assert(QUANTIZATION_MATRIX[i] != 0);
    assert(block[i] > -2048 && block[i] < 2048);

    int16_t tmp = 2 * block[i] / QUANTIZATION_MATRIX[i];

    block[i] = tmp >> 1;

    if (tmp > 0) {
      if ((tmp & 1) == 1) ++block[i];
    } else {
      if ((tmp & 0) == 1) --block[i];
    }
  }
}


void print_bits(uint16_t bits, uint8_t length) {
  for (int8_t i = length - 1; i >= 0; --i)
    if (bits & (1 << i))
      printf("1");
    else
      printf("0");
}


void tjpeg_add_to_buffer(jpeg_proc_t *proc, uint16_t bits, uint8_t length) {
  int8_t shift = 0; /* right--positive, left--negative */

  assert(length > 0 && length <= 16);
  assert(proc->bit_offset >= 0 && proc->bit_offset < 8);
  assert(proc->write_offset >= 0 && proc->write_offset < sizeof(proc->buffer));
  assert(proc->read_offset >= 0 && proc->read_offset < sizeof(proc->buffer));

  bits &= (1 << length) - 1;
  shift = (-8 + proc->bit_offset + length);

  printf("  adding ");
  print_bits(bits, length);
  printf(" to buffer (%d bits)\n", length);

  while (shift >= 0) {
    printf("  offset = %d\n", proc->bit_offset);
    printf("  shift = %d\n", shift);
    printf("  length = %d\n", length);

    assert(length > 0 && length <= 16);
    assert(proc->write_offset >= 0 && proc->write_offset < sizeof(proc->buffer));
    assert(proc->read_offset >= 0 && proc->read_offset < sizeof(proc->buffer));

    proc->buffer[proc->write_offset] |= (uint8_t) bits >> shift;
    ++proc->write_offset;
    if (proc->write_offset == sizeof(proc->buffer))
      proc->write_offset = 0;
    proc->buffer[proc->write_offset] = 0;
    /* Shifted bytes were not added */
    length = shift;
    /* Bit offset is now 0 */
    proc->bit_offset = 0;
    shift = (-8 + length);
  }

  printf("  offset = %d\n", proc->bit_offset);
  printf("  shift = %d\n", shift);
  printf("  length = %d\n", length);

  assert(proc->write_offset >= 0 && proc->write_offset < sizeof(proc->buffer));
  assert(proc->read_offset >= 0 && proc->read_offset < sizeof(proc->buffer));
  assert(shift < 0 && shift >= -8);

  proc->buffer[proc->write_offset] |= (uint8_t) bits << (-shift);
  proc->bit_offset = 8 + shift;

  assert(proc->bit_offset >= 0 && proc->bit_offset < 8);

  printf("  ");
  for (int i = proc->read_offset; 1; i = (i + 1) % sizeof(proc->buffer)) {
    if (i % 8 == 0)
      printf("\n  ");
      printf("0b");

      if (i == proc->write_offset)
        print_bits(proc->buffer[i] >> (8 - proc->bit_offset), proc->bit_offset);
      else
        print_bits(proc->buffer[i], 8);

      printf(" ");

      if (i == proc->write_offset)
        break;
  }

  printf("\n  offset = %d\n\n", proc->bit_offset);
}


uint8_t tjpeg_get_length(int16_t x) {
  uint8_t result = 0;

  if (x < 0) x = -x;

  while (x) {
    x >>= 1;
    result += 1;
  }

  return result;
}


void tjpeg_encode_y(jpeg_proc_t *p, int16_t *block)
{
  uint16_t value;
  uint8_t  length;
  uint16_t huffman_code;

  printf("Encoding Y channel\n");
  printf("Encoding DC coef\n");

  if (block[0] - p->last_y_dc > 0)
    value = block[0] - p->last_y_dc;
  else
    value = block[0] - p->last_y_dc - 1;

  length = tjpeg_get_length(value);
  huffman_code = CODE_TO_HUF_Y_DC[length];

  printf("last dc = %d, current dc = %d\n", p->last_y_dc, block[0]);
  tjpeg_add_to_buffer(p, (uint16_t) (huffman_code >> 4), (uint8_t) (huffman_code & 0x000f));
  p->last_y_dc = block[0];

  tjpeg_add_to_buffer(p, value & ((1 << length) - 1), length);

  for (uint8_t i = 1; i < 64; ++i) {
    uint8_t run = 0;

    printf("Current pos = %d (zigzaged = %d)\n", i, ZIGZAG[i]);

    while (i < 64 && block[ZIGZAG[i]] == 0) {
      ++i;
      ++run;
    }

    if (i == 64) {
      printf("End of block\n");
      tjpeg_add_to_buffer(p, (uint16_t) (CODE_TO_HUF_Y_AC[0][0] >> 4),
        (uint8_t) (CODE_TO_HUF_Y_AC[0][0] & 0x000f));
      break;
    } else {
      while (run >= 15) {
        printf("Adding run of 16 zeroes\n");
        tjpeg_add_to_buffer(p, (uint16_t) (CODE_TO_HUF_Y_AC[15][0] >> 4),
          (uint8_t) (CODE_TO_HUF_Y_AC[15][0] & 0x000f));
        run -= 16;
      }
    }

    length = tjpeg_get_length(block[ZIGZAG[i]]);
    huffman_code = CODE_TO_HUF_Y_AC[run][length];

    printf("run = %d;  value = %d;  length = %d\n", run, block[ZIGZAG[i]], length);
    printf("code = 0x%02x; huffman = 0x%04x\n", run * 16 + length, huffman_code >> 4);

    tjpeg_add_to_buffer(p, (uint16_t) (huffman_code >> 4), (uint8_t) (huffman_code & 0x000f));

    if (block[ZIGZAG[i]] > 0)
      value = block[ZIGZAG[i]];
    else
      value = block[ZIGZAG[i]] - 1;

    printf("value = %04x (%d bits)\n", value & ((1 << length) - 1), length);

    tjpeg_add_to_buffer(p, value, length);
  }
}


void tjpeg_compress_block(int16_t *block)
{
  /* Compute DCT-II */
  print_int16(block);
  reference_dct(block);
  print_int16(block);
  /* Apply quantization matrix */
  tjpeg_quantize(block);
  print_int16(block);
}


void tjpeg_init(jpeg_proc_t *processor, uint16_t width, uint16_t height)
{
  printf("Creating new processor with size %dx%d.\n", width, height);
  p->image_width = width;
  p->image_height = height;
  p->data = NULL;
  p->last_y_dc = 0;
  p->last_cr_dc = 0;
  p->last_cb_dc = 0;
  p->read_offset = 0;
  p->write_offset = 0;
  p->bit_offset = 0;
  p->buffer[0] = 0;
  p->x_pos = 0;
  p->y_pos = 0;
  p->data_x_pos = 0;
}


void tjpeg_feed_data(jpeg_proc_t *p, uint16_t width, uint8_t *data)
{
  printf("Feeding new data to processor (%d columns)\n", width);
  p->width = width;
  p->data = data;
  p->data_x_pos = 0;
}


int tjpeg_write(jpeg_proc_t *p, uint8_t* buffer, int buffer_size)
{
  int     bytes_written = 0;
  int16_t block[64];

  while (bytes_written < buffer_size) {
    /* Check if there is data available in internal buffer */
    if (p->read_offset > p->write_offset || p->read_offset < p->write_offset) {
      int8_t to_read = p->write_offset - p->read_offset;
      int8_t a;

      if (to_read < 0) to_read += sizeof(p->buffer);
      if (to_read + bytes_written > buffer_size) to_read = buffer_size - bytes_written;

      if (p->read_offset + to_read > sizeof(p->buffer)) {
        a = sizeof(p->buffer) - p->read_offset;
        memcpy(buffer + bytes_written, p->buffer + to_read, a);
        memcpy(buffer + bytes_written + a, p->buffer, to_read - a);
        p->read_offset = to_read - a;
      } else {
        memcpy(buffer + bytes_written, p->buffer + to_read, to_read);
        p->read_offset += to_read;
      }

      bytes_written += to_read;
    }
    else /* Produce more data */
    {
      if (p->x_pos >= p->width && p->y_pos >= p->height) {
        printf("End of image!\n");
        break;
      }

      for (uint8_t y = 0; y < 8; ++y)
        for (uint8_t x = 0; x < 8; ++x) {
          block[x + y * 8] = p->data[2 * (p->data_offset + x + y * p->width)] - 128;
        }

      p->data_offset += 8;

      tjpeg_compress_block(block);
      tjpeg_encode_y(p, block);
    }
  }


  return bytes_written;
}
