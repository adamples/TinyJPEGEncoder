#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tjpeg_image_chunk.h"


int _rand(int min, int max)
{
  return min + (rand() % (max - min + 1));
}


void test_next_block()
{
  tjpeg_image_chunk_t ic;

  for (int width = 1; width < 2000; width += _rand(0, 31))
    for (int height = 1; height < 2000; height += _rand(0, 31)) {
      tjpeg_image_chunk_init(&ic, NULL, width, height);

      int blocks_n = ((width - 1) / 16 + 1) * ((height - 1) / 8 + 1);

      printf("%dx%d = %d block(s)\n", width, height, blocks_n);

      for (int i = 0; i < blocks_n; ++i)
        assert(tjpeg_image_chunk_next_block(&ic));

      for (int i = 0; i < 10; ++i)
        assert(!tjpeg_image_chunk_next_block(&ic));
    }
}


void test_copy()
{
}


int main(void)
{
  srand(time(0));

  for (int i = 0; i < 10; ++i) {
    test_next_block();
  }
}
