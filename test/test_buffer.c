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
  tjpeg_buffer_t *b = tjpeg_buffer_new();
  int             l = 0;

  printf("test_length\n");

  while (1) {
    int r = _rand(1, 16);

    printf("expected length: %d\n", l / 8);
    printf("expected bit length: %d\n", l);

    assert(tjpeg_buffer_get_length(b) == l / 8);

    l += r;

    if (l <= TJPEG_BUFFER_SIZE * 8)
      tjpeg_buffer_add(b, _rand(0x0000, 0xffff) & ((1 << r) - 1), r);
    else
      break;
  }
}


void test_content()
{
  tjpeg_buffer_t *b = tjpeg_buffer_new();
  uint8_t         data[TJPEG_BUFFER_SIZE * 8];
  int             p = 0;

  printf("test_content\n");

  tjpeg_buffer_init(b);

  for (int i = 0; i < sizeof(data); ++i)
    data[i] = _rand(0, 1);

  while (p < sizeof(data) - 24) {
    int r = _rand(1, 16);
    uint16_t bits = 0;

    for (int i = 0; i < r; ++i)
      bits |= (1 & data[p + i]) << (r - 1 - i);

    tjpeg_buffer_add(b, bits, r);
    p += r;
  }

  int r = sizeof(data) - p - 8;
  uint16_t bits = 0;

  for (int i = 0; i < r; ++i)
    bits |= (1 & data[p + i]) << (r - 1 - i);

  tjpeg_buffer_add(b, bits & ((1 << r) - 1), r);
  p += r;

  for (int i = 0; i < TJPEG_BUFFER_SIZE - 1; ++i) {
    uint8_t bits = 0;

    for (int j = 0; j < 8; ++j) {
      bits |= (1 & data[i * 8 + j]) << (7 - j);
    }

    printf("Checking byte %2d:  0x%02x == 0x%02x\n", i, bits, ((unsigned char *) b)[i]);
    assert(bits == ((unsigned char *) b)[i]);
  }
}


int main(void)
{
  srand(time(0));

  //for (int i = 0; i < 10; ++i) {
    //test_length();
    test_content();
    //~ test_copy();
  //}

}
