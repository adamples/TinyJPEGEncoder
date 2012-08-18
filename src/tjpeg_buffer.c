#include "tjpeg_buffer.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>


void print_bits(uint16_t bits, uint8_t length)
{
  for (int8_t i = length - 1; i >= 0; --i)
    if (bits & (1 << i))
      printf("1");
    else
      printf("0");
}


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
  buffer->read_offset = 0;
  buffer->write_offset = 0;
  buffer->bit_offset = 0;
  buffer->data[0] = 0;
}


uint16_t tjpeg_buffer_get_length(tjpeg_buffer_t *b)
{
  assert(b != NULL);

  if (b->read_offset <= b->write_offset)
    return b->write_offset - b->read_offset;
  else
    return TJPEG_BUFFER_SIZE - (b->read_offset - b->write_offset);
}


uint16_t tjpeg_buffer_get_bitlength(tjpeg_buffer_t *b)
{
  assert(b != NULL);

  return tjpeg_buffer_get_length(b) * 8 + b->bit_offset;
}


void tjpeg_buffer_add(tjpeg_buffer_t *buffer, uint16_t bits, uint8_t length)
{
  int8_t shift = 0; /* right--positive, left--negative */

  assert(buffer != NULL);
  assert(length > 0 && length <= 16);
  assert(buffer->bit_offset >= 0 && buffer->bit_offset < 8);
  assert(buffer->write_offset >= 0 && buffer->write_offset < sizeof(buffer->data));
  assert(buffer->read_offset >= 0 && buffer->read_offset < sizeof(buffer->data));

  bits &= (1 << length) - 1;
  shift = (-8 + buffer->bit_offset + length);

  printf("  adding ");
  print_bits(bits, length);
  printf(" to buffer (%d bits)\n", length);

  while (shift >= 0) {
    printf("  offset = %d\n", buffer->bit_offset);
    printf("  shift = %d\n", shift);
    printf("  length = %d\n", length);

    assert(length > 0 && length <= 16);
    assert(buffer->write_offset >= 0 && buffer->write_offset < sizeof(buffer->data));
    assert(buffer->read_offset >= 0 && buffer->read_offset < sizeof(buffer->data));

    buffer->data[buffer->write_offset] |= (uint8_t) (bits >> shift);
    if (buffer->data[buffer->write_offset] == 0xff) {
      printf("Adding stuff byte\n");
      ++buffer->write_offset;
      if (buffer->write_offset == sizeof(buffer->data))
        buffer->write_offset = 0;
      buffer->data[buffer->write_offset] = 0;
      assert(buffer->write_offset != buffer->read_offset);
    }
    ++buffer->write_offset;
    if (buffer->write_offset == sizeof(buffer->data))
      buffer->write_offset = 0;
    buffer->data[buffer->write_offset] = 0;
    assert(buffer->write_offset != buffer->read_offset);
    /* Shifted bytes were not added */
    length = shift;
    /* Bit offset is now 0 */
    buffer->bit_offset = 0;
    shift = (-8 + length);
  }

  printf("  offset = %d\n", buffer->bit_offset);
  printf("  shift = %d\n", shift);
  printf("  length = %d\n", length);

  assert(buffer->write_offset >= 0 && buffer->write_offset < sizeof(buffer->data));
  assert(buffer->read_offset >= 0 && buffer->read_offset < sizeof(buffer->data));
  assert(shift < 0 && shift >= -8);

  buffer->data[buffer->write_offset] |= (uint8_t) bits << (-shift);
  buffer->bit_offset = 8 + shift;

  assert(buffer->bit_offset >= 0 && buffer->bit_offset < 8);

  printf("  ");
  for (int i = buffer->read_offset; 1; i = (i + 1) % sizeof(buffer->data)) {
    if (buffer->data[i] == 0xff) {
      //printf("undetected marker\n");
      //exit(0);
    }
    if (i % 8 == 0)
      printf("\n  ");
      printf("0b");

      if (i == buffer->write_offset)
        print_bits(buffer->data[i] >> (8 - buffer->bit_offset), buffer->bit_offset);
      else
        print_bits(buffer->data[i], 8);

      printf(" ");

      if (i == buffer->write_offset)
        break;
  }

  printf("\n  offset = %d\n\n", buffer->bit_offset);
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

  printf("Adding AC value: %d (run: %d; length: %d; symbol: %02x; code: %04x)\n",
    value, run, length, run * 16 + length, code);

  if ((code & 0x000f) > 11)
    tjpeg_buffer_add(buffer, (code >> 4) | 0xf000, (code & 0x000f) + 1);
  else
    tjpeg_buffer_add(buffer, code >> 4, (code & 0x000f) + 1);

  if (length != 0)
    tjpeg_buffer_add(buffer, uvalue, length);
  else
    printf("EOB marker, not adding value\n");
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

  printf("Adding DC value: %d (%04x) (length: %d; symbol: %02x; code: %04x)\n",
    value, uvalue, length, length, code);

  tjpeg_buffer_add(buffer, code >> 4, code & 0x000f);

  if (length != 0)
    tjpeg_buffer_add(buffer, uvalue, length);
  else
    printf("DC value not changed, not adding value\n");
}


void tjpeg_buffer_copy(tjpeg_buffer_t *b, uint8_t *destination, int bytes_n)
{
  assert(bytes_n <= tjpeg_buffer_get_length(b));

  printf("copy (%d) bytes from buffer of size %d\n", bytes_n, tjpeg_buffer_get_length(b));

  if (b->read_offset < b->write_offset) {
    memcpy(destination, b->data + b->read_offset, bytes_n);
    b->read_offset += bytes_n;
  } else {
    int at_end = TJPEG_BUFFER_SIZE - b->read_offset;

    printf("at_end = %d\n", at_end);

    if (at_end >= bytes_n) {
      printf("copy (%d) bytes to (destination) from (b->data + %d)\n", bytes_n, b->read_offset);
      memcpy(destination, b->data + b->read_offset, bytes_n);
      b->read_offset += bytes_n;
      if (b->read_offset >= TJPEG_BUFFER_SIZE) b->read_offset = 0;
    } else {
      printf("b->read_offset >= b->write_offset (%d %d)\n", b->read_offset, b->write_offset);
      printf("copy (%d) bytes to (destination) from (b->data + %d)\n", at_end, b->read_offset);
      memcpy(destination, b->data + b->read_offset, at_end);
      printf("copy (%d) bytes to (destination + %d) from (b->data)\n", bytes_n - at_end, at_end);
      memcpy(destination + at_end, b->data, bytes_n - at_end);
      b->read_offset = bytes_n - at_end;
    }
  }
}


void tjpeg_buffer_add_byte(tjpeg_buffer_t *b, uint8_t byte)
{
  if (b->bit_offset != 0) {
    b->write_offset += 1;
    b->bit_offset = 0;

    if (b->write_offset >= sizeof(b->data))
      b->write_offset = 0;
  }

  b->data[b->write_offset] = byte;
  b->write_offset += 1;

  if (b->write_offset >= sizeof(b->data))
    b->write_offset = 0;
}
