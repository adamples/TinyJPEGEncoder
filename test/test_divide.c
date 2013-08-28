#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int reference_divide(int a, int b)
{
  int tmp = 2 * a / b;

  if (tmp > 0)
    if (tmp % 2 == 1)
      return tmp / 2 + 1;
    else
      return tmp / 2;
  else
    if (tmp % 2 == -1)
      return tmp / 2 - 1;
    else
      return tmp / 2;
}

int fast_divide(int a, int b)
{
  int t = 2 * a / b;

  switch (t & 0x80000001) {
    case 0x80000001 : return t / 2 - 1;
    case 0x00000001 : return t / 2 + 1;
    default : return t / 2;
  }
}

int main(void)
{
  srand(time(NULL));

  for (int i = 0; i < 100; ++i) {
    int a = rand() % 201 - 100;
    int b = (rand() % 100 + 1) * ((rand() % 2) * 2 - 1);
    int rd = reference_divide(a, b);
    int fd = fast_divide(a, b);

    if (rd != fd)
      printf("%4d / %4d = %7.3f;   reference = %3d;   fast = %3d;   result = %s\n",
        a, b, (double) a / b, rd, fd, rd == fd ? "OK" : "FAIL");
  }

  for (int i = 0; i < 100000000; ++i) {
    int r = fast_divide(rand() - 0x7fffffff, rand() - 0x7fffffff);
  }
}
