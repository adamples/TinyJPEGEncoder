#include "dct.h"

#include <stdint.h>
#include <math.h>


double alpha(int8_t u) {
  const double sqrt_1_8 = sqrt(0.125);
  const double sqrt_2_8 = sqrt(0.250);

  if (u == 0)
    return sqrt_1_8;
  else
    return sqrt_2_8;
}


void reference_dct(int16_t *data)
{
  double tmp[64];

  for (int8_t v = 0; v < 8; ++v)
    for (int8_t u = 0; u < 8; ++u) {
      tmp[u + v * 8] = 0;

      for (int8_t y = 0; y < 8; ++y)
        for (int8_t x = 0; x < 8; ++x) {
          tmp[u + v * 8] += alpha(u) * alpha(v) * ((double) data[x + y * 8]) *
            cos(M_PI * u * (x + 0.5) / 8) * cos(M_PI * v * (y + 0.5) / 8);
        }
    }

  for (int8_t y = 0; y < 8; ++y)
    for (int8_t x = 0; x < 8; ++x)
      data[x + y * 8] = tmp[x + y * 8];
}
