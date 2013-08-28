#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include "dct.h"


int _rand(int min, int max)
{
  return min + (rand() % (max - min + 1));
}


void test_fastest_dct_block_butterfly()
{
  fix_int_t a, b;

  a = 1;
  b = 1;
  fastest_dct_block_butterfly(&a, &b);
  assert(a == 2);
  assert(b == 0);

  a = 1;
  b = -1;
  fastest_dct_block_butterfly(&a, &b);
  assert(a == 0);
  assert(b == 2);

  a = -1;
  b = 1;
  fastest_dct_block_butterfly(&a, &b);
  assert(a == 0);
  assert(b == -2);
}


void test_fastest_dct_block_rotation()
{
  fix_int_t a, b;

  a = 7;
  b = -13;
  fastest_dct_block_rotation_1_3(&a, &b);
  printf("a : %5d;  b : %5d\n", a, b);
  assert(a == -1);
  assert(b == -14);

  a = 70;
  b = -130;
  fastest_dct_block_rotation_1_3(&a, &b);
  printf("a : %5d;  b : %5d\n", a, b);
  assert(a == -14);
  assert(b == -146);

  a = -44;
  b = -96;
  fastest_dct_block_rotation_1_1(&a, &b);
  printf("a : %5d;  b : %5d\n", a, b);
  assert(a == -61);
  assert(b == -85);
}


void reference_dct_1d(fix_int_t *data)
{
  double input[8];
  double output[8];

  for (int i = 0; i < 8; ++i)
    input[i] = data[i];

  for (int k = 0; k < 8; ++k) {
    output[k] = 0;

    for (int n = 0; n < 8; ++n)
      output[k] += input[n] * cos(M_PI * ((double) n + 0.5) * k / 8);
  }

  for (int i = 0; i < 8; ++i)
    data[i] = output[i];
}


void test_fastest_dct_1d_x()
{
  fix_int_t data_a[8] = {
    74, 92, -8, -100, 10, -57, 1, 15
  };

  fix_int_t data_b[8] = {
    74, 92, -8, -100, 10, -57, 1, 15
  };

  reference_dct_1d(data_a);

  for (int i = 0; i < 8; ++i)
    printf("%6d", data_a[i]);
  printf("\n");

  fastest_dct_1d_x(data_b);

  for (int i = 0; i < 8; ++i)
    printf("%6d", data_b[i]);
  printf("\n");
}



int main(void)
{
  srand(time(0));

  //test_fastest_dct_block_butterfly();
  //test_fastest_dct_block_rotation();

//  for (int i = 0; i < 10; ++i) {
    test_fastest_dct_1d_x();
//  }
}
