#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tjpeg_buffer.h"


int _rand(int min, int max)
{
  return min + (rand() % (max - min + 1));
}


void test_length()
{
  tjpeg_buffer_t  b;
  int             l = 0;

  printf("test_length\n");

  tjpeg_buffer_init(&b);

  while (l < TJPEG_BUFFER_SIZE * 8) {
    int r = _rand(1, 16);

    printf("expected length: %d\n", l / 8);
    printf("expected bit length: %d\n", l);

    assert(tjpeg_buffer_get_length(&b) == l / 8);
    assert(tjpeg_buffer_get_bitlength(&b) == l);

    l += r;
    tjpeg_buffer_add(&b, _rand(0x0000, 0xffff), r);
  }
}


void test_content()
{
  tjpeg_buffer_t  b;
  uint8_t         data[TJPEG_BUFFER_SIZE * 8];
  int             p = 0;

  printf("test_content\n");

  tjpeg_buffer_init(&b);

  for (int i = 0; i < sizeof(data); ++i)
    data[i] = _rand(0, 1);

  while (p < sizeof(data) - 24) {
    int r = _rand(1, 16);
    uint16_t bits = 0;

    for (int i = 0; i < r; ++i)
      bits |= (1 & data[p + i]) << (r - 1 - i);

    tjpeg_buffer_add(&b, bits, r);
    p += r;
  }

  int r = sizeof(data) - p - 8;
  uint16_t bits = 0;

  for (int i = 0; i < r; ++i)
    bits |= (1 & data[p + i]) << (r - 1 - i);

  tjpeg_buffer_add(&b, bits, r);
  p += r;

  for (int i = 0; i < TJPEG_BUFFER_SIZE - 1; ++i) {
    uint8_t bits = 0;

    for (int j = 0; j < 8; ++j) {
      bits |= (1 & data[i * 8 + j]) << (7 - j);
    }

    printf("Checking byte %2d:  0x%02x == 0x%02x\n", i, bits, b.data[i]);
    assert(bits == b.data[i]);
  }
}


void test_copy()
{
  tjpeg_buffer_t  b;
  uint8_t         data[TJPEG_BUFFER_SIZE * 8];
  int             p = 0;
  uint8_t         copied[TJPEG_BUFFER_SIZE];
  int             copied_n = 0;

  printf("test_content\n");

  tjpeg_buffer_init(&b);

  for (int l = 0; l < 128; ++l) {
    for (int i = 0; i < sizeof(data); ++i)
      data[i] = _rand(0, 1);

    while (p < sizeof(data) - 24) {
      int r = _rand(1, 16);
      uint16_t bits = 0;

      for (int i = 0; i < r; ++i)
        bits |= (1 & data[p + i]) << (r - 1 - i);

      tjpeg_buffer_add(&b, bits, r);
      p += r;
      assert(tjpeg_buffer_get_bitlength(&b) == p);
    }

    int r = sizeof(data) - p - 8;
    uint16_t bits = 0;

    for (int i = 0; i < r; ++i)
      bits |= (1 & data[p + i]) << (r - 1 - i);

    tjpeg_buffer_add(&b, bits, r);
    p += r;
    assert(tjpeg_buffer_get_bitlength(&b) == p);

    assert(tjpeg_buffer_get_bitlength(&b) == p);
    tjpeg_buffer_copy(&b, copied, 10);
    assert(tjpeg_buffer_get_bitlength(&b) == p - 80);
    tjpeg_buffer_copy(&b, copied + 10, 5);
    assert(tjpeg_buffer_get_bitlength(&b) == p - 120);
    tjpeg_buffer_copy(&b, copied + 15, 16);
    assert(tjpeg_buffer_get_bitlength(&b) == p - 248);
    tjpeg_buffer_copy(&b, copied + 31, TJPEG_BUFFER_SIZE - 10 - 5 - 16 - 1);
    assert(tjpeg_buffer_get_bitlength(&b) == 0);

    for (int i = 0; i < TJPEG_BUFFER_SIZE - 1; ++i) {
      uint8_t bits = 0;

      for (int j = 0; j < 8; ++j) {
        bits |= (1 & data[i * 8 + j]) << (7 - j);
      }

      printf("Checking byte %2d:  0x%02x == 0x%02x\n", i, bits, copied[i]);
      assert(bits == copied[i]);
    }

    p = 0;
    assert(tjpeg_buffer_get_bitlength(&b) == p);
  }
}


int main(void)
{
  srand(time(0));

  for (int i = 0; i < 10; ++i) {
    test_length();
    test_content();
    test_copy();
  }

}
