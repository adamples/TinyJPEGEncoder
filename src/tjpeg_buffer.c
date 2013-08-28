#include "tjpeg_buffer.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


struct _tjpeg_buffer_t {
  unsigned char  data[TJPEG_BUFFER_SIZE];  /** Buffer data */
  unsigned int  bit_size;  /**  Size of buffer in bits */
};


uint8_t tjpeg_get_length(int16_t x)
{
  uint8_t result = 0;

  if (x < 0) x = -x;

  while (x) {
    x >>= 1;
    result += 1;
  }

  return result;
}


void tjpeg_buffer_init(tjpeg_buffer_t *buffer)
{
  buffer->bit_size = 0;
  *((int *) buffer) = 0;
}


tjpeg_buffer_t *tjpeg_buffer_new(void)
{
  tjpeg_buffer_t *buffer = (tjpeg_buffer_t *) malloc(sizeof(tjpeg_buffer_t));
  tjpeg_buffer_init(buffer);
  return buffer;
}


unsigned char *tjpeg_buffer_get_data(tjpeg_buffer_t *b)
{
  assert(b != NULL);
  return b->data;
}


int tjpeg_buffer_get_length(tjpeg_buffer_t *b)
{
  assert(b != NULL);
  return b->bit_size >> 3;
}


/*
 * Appends certain number of bits stored in integer in little-endian order.
 *
 * @example
 * Little endian, 32-bit: add 13 bits to buffer of length 22:
 *
 * Buffer:
 * |   00   |   01   |   02   |   03   |   04   |   05   |   06   |   07   |
 * |hgfedcba|ponmlkji|__vutsrq|________|________|________|________|________|
 *
 * Bits to add:
 * |   00   |   01   |   02   |   03   |
 * |HGFEDCBA|___MLKJI|________|________|
 * SHL 6 (bit_size % 8 = bit_size & 0x07)
 * |   00   |   01   |   02   |   03   |
 * |BA______|JIHGFEDC|_____MLK|________|
 *
 *                    pointer:
 * |   00   |   01   |   02   |   03   |   04   |   05   |   06   |   07   |
 * |hgfedcba|ponmlkji|__vutsrq|________|________|________|________|________|
 * - OR --------------------------------------------------------------------
 *                   |   00   |   01   |   02   |   03   |
 *                   |BA______|JIHGFEDC|_____MLK|________|
 * - = ---------------------------------------------------------------------
 * |   00   |   01   |   02   |   03   |   04   |   05   |   06   |   07   |
 * |hgfedcba|ponmlkji|BAvutsrq|JIHGFEDC|_____MLK|________|________|________|
 */


void buffer_print(tjpeg_buffer_t *buffer)
{
  printf("\n\nBuffer of size %d:\n  ", buffer->bit_size);

  for (int i = 0; i < buffer->bit_size / 8 + 1; ++i) {
    for (int j = 7; j >= 0; j -= 1)
      if (i * 8 + (7 - j) < buffer->bit_size)
        printf("%d", (buffer->data[i] & (1 << j)) ? 1 : 0);
      else
        printf("_");
    printf((i % 8 == 7) ? "\n  " : " ");
  }

  printf("\n\n");
}


/*
 * Appends certain number of bits stored in integer in little-endian order.
 *
 * @example
 * Little endian, 32-bit: add 13 bits to buffer of length 22:
 *
 * Buffer:
 * |   00   |   01   |   02   |   03   |   04   |   05   |   06   |   07   |
 * |abcdefgh|ijklmnop|qrstuv__|________|________|________|________|________|
 *
 * Bits to add:
 * |   00   |   01   |   02   |   03   |
 * |FGHIJKLM|___ABCDE|________|________|
 * SHL 13: 32 - length - bit_size % 8 = 32 - 13 - 6 = 13
 * |   00   |   01   |   02   |   03   |
 * |________|KLM_____|CDEFGHIJ|______AB|
 */
void tjpeg_buffer_add_le(tjpeg_buffer_t *buffer, unsigned int bits, unsigned char length)
{
  unsigned char *wp;
  unsigned char *rp;

  bits = bits << (sizeof(bits) * 8 - length - (buffer->bit_size & 0x07));
  wp = buffer->data + (buffer->bit_size >> 3);
  rp = ((unsigned char *) &bits) + sizeof(bits) - 1;

  for (int i = 0; i < length + 8; i += 8, ++wp, --rp, *wp = 0) {
    *wp |= *rp;
    if (*wp == 0xff) {
      ++wp;
      buffer->bit_size += 8;
      *wp = 0;
    }
  }

  //pointer = (unsigned int *) (buffer->data + (buffer->bit_size >> 3));
  //*pointer |= bits << (buffer->bit_size & 0x07);
  //*pointer &= ~(bits << (buffer->bit_size & 0x07));

#ifndef NDEBUG
  //~ unsigned int shifted_bits = bits << (buffer->bit_size & 0x07);
  //~ unsigned int mask = (1 << length) - 1;
  //~ unsigned int shifted_mask = mask << (buffer->bit_size & 0x07);

  //~ printf("Added %d bits: 0x%08x;  mask: 0x%08x;  shift: %d\n", length, bits, mask, buffer->bit_size & 0x07);
  //~ printf("shifted bits: 0x%08x;  shifted mask: 0x%08x\n", shifted_bits, shifted_mask);
  //~ printf("dereferenced pointer: 0x%08x\n", *pointer);
  //~ printf("0x%08x =?= 0x%08x\n", *pointer & shifted_mask, shifted_bits & shifted_mask);

  //~ assert((*pointer & shifted_mask) == (shifted_bits & shifted_mask));
#endif

  buffer->bit_size += length;
  //buffer_print(buffer);
}


void tjpeg_buffer_add(tjpeg_buffer_t *buffer, unsigned int bits, unsigned char length)
{
  assert(buffer != NULL);
  assert(length <= sizeof(bits) * 8 - 8);
  assert((buffer->bit_size + length) / 8 <= TJPEG_BUFFER_SIZE);
  assert((bits & ((1 << length) - 1)) == bits);

  //~ for (int i = length - 1; i >= 0; --i)
    //~ printf("%d", bits & (1 << i) ? 1 : 0);

#ifdef TJPEG_BIG_ENDIAN
  tjpeg_buffer_add_be(buffer, bits, length);
#else
  tjpeg_buffer_add_le(buffer, bits, length);
#endif
}


void tjpeg_buffer_add_ac(tjpeg_buffer_t *buffer, const uint16_t* table, uint8_t run, int16_t value)
{
  assert(buffer != NULL);
  assert(table != NULL);
  assert(run >= 0 && run < 16);
  assert(value >= -2048 && value <= 2048);

  uint16_t  uvalue = value < 0 ? value - 1 : value;
  uint8_t   length = tjpeg_get_length(value);
  uint16_t  code = table[length * 16 + run];

  assert(length < 12);
  assert(code != 0);

  //~ printf("\033[34mAC:%d/%d:\033[0m", run, value);

  if ((code & 0x000f) > 11)
    tjpeg_buffer_add(buffer, (code >> 4) | (0x0f00 << ((code & 0x000f) - 11)), (code & 0x000f) + 1);
  else
    tjpeg_buffer_add(buffer, code >> 4, (code & 0x000f) + 1);

  //~ printf("\033[34m:\033[0m");

  if (length != 0)
    tjpeg_buffer_add(buffer, uvalue & ((1 << length) - 1), length);

  //~ printf("\033[34m|\033[0m");
}


void tjpeg_buffer_add_dc(tjpeg_buffer_t *buffer, const uint16_t *table, int16_t value)
{
  assert(buffer != NULL);
  assert(table != NULL);
  assert(value >= -2048 && value <= 2048);

  uint16_t  uvalue = value < 0 ? value - 1 : value;
  uint8_t   length = tjpeg_get_length(value);
  uint16_t  code = table[length];

  assert(length < 12);
  assert(code != 0);

  //~ printf("\033[34mDC:%d:\033[0m", value);

  tjpeg_buffer_add(buffer, code >> 4, code & 0x000f);

  //~ printf("\033[34m:\033[0m");

  if (length != 0)
    tjpeg_buffer_add(buffer, uvalue & ((1 << length) - 1), length);

  //~ printf("\033[34m|\033[0m");
}


void tjpeg_buffer_add_byte(tjpeg_buffer_t *b, unsigned char byte)
{
  b->bit_size = ((b->bit_size + 15) / 8) * 8;
  b->data[b->bit_size / 8] = byte;
}


void tjpeg_buffer_trunc_bytes(tjpeg_buffer_t *b)
{
  b->data[0] = b->data[b->bit_size / 8];
  b->bit_size &= 0x07;
}
