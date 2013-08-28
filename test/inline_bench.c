#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>


inline void mul_inline(int *a, int *b)
{
  *a = *a * *b;
  *b = *a + *b;
}

void mul_func(int *a, int *b)
{
  *a = *a * *b;
  *b = *a + *b;
}


int main(void)
{
  int a = (int) main;
  int b = (int) mul_func;
  int t[2] = { (int) main, (int) mul_func };

  mul_func(t, t + 1);
  printf("%d %d\n", t[0], t[1]);
  mul_inline(t, t + 1);
  printf("%d %d\n", t[0], t[1]);
  t[0] = t[0] * t[1];
  t[1] = t[1] + t[1];
  printf("%d %d\n", t[0], t[1]);
  return 0;
}
