#include "tjpeg_buffer.h"

#include <assert.h>
#include <string.h>


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

  while (shift >= 0) {
    assert(length > 0 && length <= 16);
    assert(buffer->write_offset >= 0 && buffer->write_offset < sizeof(buffer->data));
    assert(buffer->read_offset >= 0 && buffer->read_offset < sizeof(buffer->data));

    buffer->data[buffer->write_offset] |= (uint8_t) (bits >> shift);
    if (buffer->data[buffer->write_offset] == 0xff) {
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

  assert(buffer->write_offset >= 0 && buffer->write_offset < sizeof(buffer->data));
  assert(buffer->read_offset >= 0 && buffer->read_offset < sizeof(buffer->data));
  assert(shift < 0 && shift >= -8);

  buffer->data[buffer->write_offset] |= (uint8_t) bits << (-shift);
  buffer->bit_offset = 8 + shift;

  assert(buffer->bit_offset >= 0 && buffer->bit_offset < 8);
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

  if ((code & 0x000f) > 11)
    tjpeg_buffer_add(buffer, (code >> 4) | 0xf000, (code & 0x000f) + 1);
  else
    tjpeg_buffer_add(buffer, code >> 4, (code & 0x000f) + 1);

  if (length != 0)
    tjpeg_buffer_add(buffer, uvalue, length);
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

  tjpeg_buffer_add(buffer, code >> 4, code & 0x000f);

  if (length != 0)
    tjpeg_buffer_add(buffer, uvalue, length);
}


void tjpeg_buffer_copy(tjpeg_buffer_t *b, uint8_t *destination, int bytes_n)
{
  assert(bytes_n <= tjpeg_buffer_get_length(b));

  if (b->read_offset < b->write_offset) {
    memcpy(destination, b->data + b->read_offset, bytes_n);
    b->read_offset += bytes_n;
  } else {
    int at_end = TJPEG_BUFFER_SIZE - b->read_offset;

    if (at_end >= bytes_n) {
      memcpy(destination, b->data + b->read_offset, bytes_n);
      b->read_offset += bytes_n;
      if (b->read_offset >= TJPEG_BUFFER_SIZE) b->read_offset = 0;
    } else {
      memcpy(destination, b->data + b->read_offset, at_end);
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
