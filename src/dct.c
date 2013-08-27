#include "dct.h"

#include <stdint.h>
#include <math.h>

//~ #define SIN_1_1     (1.950903220e-1)
//~ #define COS_1_1     (9.807852804e-1)
//~ #define SIN_1_3     (5.555702330e-1)
//~ #define COS_1_3     (8.314696123e-1)
//~ #define SIN_SQRT2_6 (1.306562965e+0)
//~ #define COS_SQRT2_6 (5.411961001e-1)
//~ #define SQRT_2      (1.414213562e+0)
//~ #define SQRT_8      (2.828427125e+0)


#define FIX_BITS      (sizeof(fix_int_t) * 8 - 14)
#define FIX_1         (1 << (FIX_BITS))
#define FIX_HALF      (1 << (FIX_BITS / 2))
#define FIX(f)        ((fix_int_t) ((f) * (FIX_1)))
#define FIX_TO_INT(f) ((f) / (FIX_1))

const fix_int_t SIN_1_1       = FIX(1.950903220e-1);
const fix_int_t COS_1_1       = FIX(9.807852804e-1);
const fix_int_t SIN_1_3       = FIX(5.555702330e-1);
const fix_int_t COS_1_3       = FIX(8.314696123e-1);
const fix_int_t SIN_SQRT2_6   = FIX(1.306562965e+0);
const fix_int_t COS_SQRT2_6   = FIX(5.411961001e-1);
const fix_int_t INV_SQRT_2    = FIX(1 / 1.414213562e+0);
const fix_int_t SQRT_2        = FIX(1.414213562e+0);
const fix_int_t SQRT_8        = FIX(2.828427125e+0);


static inline fix_int_t fix_mul(fix_int_t a, fix_int_t b)
{
  //return ((int64_t) a * b + FIX_1 / 2) / FIX_1;
  return (a / FIX_HALF) * (b / FIX_HALF);
  //~ int64_t r = (int64_t) a * b;
//~
  //~ if (r & (FIX_1 >> 1))
    //~ if (r > 0)
      //~ return (r / FIX_1) + 1;
    //~ else
      //~ return (r / FIX_1) - 1;
  //~ else
    //~ return (r / FIX_1);
}


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
      data[x + y * 8] = round(tmp[x + y * 8]);
}


//~ void fastest_dct_block_butterfly(fix_int_t *i0, fix_int_t *i1)
//~ {
  //~ fix_int_t tmp = *i0;
//~
  //~ *i0 = *i0 + *i1;
  //~ *i1 = tmp - *i1;
//~ }
//~
//~
//~ void fastest_dct_block_rotation_1_3(fix_int_t *i0, fix_int_t *i1)
//~ {
  //~ fix_int_t tmp = *i0;
//~
  //~ *i0 = (SIN_1_3 - COS_1_3) * *i1 + COS_1_3 * (tmp + *i1);
  //~ *i1 = - (COS_1_3 + SIN_1_3) * tmp + COS_1_3 * (tmp + *i1);
//~ }
//~
//~
//~ void fastest_dct_block_rotation_1_1(fix_int_t *i0, fix_int_t *i1)
//~ {
  //~ fix_int_t tmp = *i0;
//~
  //~ *i0 = (SIN_1_1 - COS_1_1) * *i1 + COS_1_1 * (tmp + *i1);
  //~ *i1 = - (COS_1_1 + SIN_1_1) * tmp + COS_1_1 * (tmp + *i1);
//~ }
//~
//~
//~ void fastest_dct_block_rotation_sqrt2_6(fix_int_t *i0, fix_int_t *i1)
//~ {
  //~ fix_int_t tmp = *i0;
//~
  //~ *i0 = (SIN_SQRT2_6 - COS_SQRT2_6) * *i1 + COS_SQRT2_6 * (tmp + *i1);
  //~ *i1 = - (COS_SQRT2_6 + SIN_SQRT2_6) * tmp + COS_SQRT2_6 * (tmp + *i1);
//~ }
//~
//~
//~ void fastest_dct_block_mult(fix_int_t *i)
//~ {
  //~ *i *= SQRT_2;
//~ }


static inline void fastest_dct_block_butterfly(fix_int_t *i0, fix_int_t *i1)
{
  fix_int_t tmp = *i0;

  *i0 = *i0 + *i1;
  *i1 = tmp - *i1;
}


static inline void fastest_dct_block_rotation_1_3(fix_int_t *i0, fix_int_t *i1)
{
  fix_int_t tmp = *i0;
  fix_int_t b = fix_mul(COS_1_3, tmp + *i1);

  *i0 = fix_mul(SIN_1_3 - COS_1_3, *i1) + b;
  *i1 = -fix_mul(COS_1_3 + SIN_1_3, tmp) + b;
}


static inline void fastest_dct_block_rotation_1_1(fix_int_t *i0, fix_int_t *i1)
{
  fix_int_t tmp = *i0;
  fix_int_t b = fix_mul(COS_1_1, tmp + *i1);

  *i0 = fix_mul(SIN_1_1 - COS_1_1, *i1) + b;
  *i1 = -fix_mul(COS_1_1 + SIN_1_1, tmp) + b;
}


static inline void fastest_dct_block_rotation_sqrt2_6(fix_int_t *i0, fix_int_t *i1)
{
  fix_int_t tmp = *i0;
  fix_int_t b = fix_mul(COS_SQRT2_6, tmp + *i1);

  *i0 = fix_mul(SIN_SQRT2_6 - COS_SQRT2_6, *i1) + b;
  *i1 = -fix_mul(COS_SQRT2_6 + SIN_SQRT2_6, tmp) + b;
}


static inline void fastest_dct_block_mult(fix_int_t *i)
{
  *i = fix_mul(*i, SQRT_2);
}


void fastest_dct_1d_x(fix_int_t *data)
{
  fix_int_t tmp;

  /* - Stage 1 ------------------------------------------------------------- */

  fastest_dct_block_butterfly(data + 0, data + 7);
  fastest_dct_block_butterfly(data + 1, data + 6);
  fastest_dct_block_butterfly(data + 2, data + 5);
  fastest_dct_block_butterfly(data + 3, data + 4);

  /* - Stage 2 ------------------------------------------------------------- */

  fastest_dct_block_butterfly(data + 0, data + 3);
  fastest_dct_block_butterfly(data + 1, data + 2);

  fastest_dct_block_rotation_1_3(data + 4, data + 7);
  fastest_dct_block_rotation_1_1(data + 5, data + 6);

  /* - Stage 3 ------------------------------------------------------------- */

  fastest_dct_block_butterfly(data, data + 1);
  fastest_dct_block_butterfly(data + 4, data + 6);
  fastest_dct_block_butterfly(data + 7, data + 5);
  fastest_dct_block_rotation_sqrt2_6(data + 2, data + 3);

  /* - Stage 4 ------------------------------------------------------------- */

  fastest_dct_block_butterfly(data + 7, data + 4);
  fastest_dct_block_mult(data + 5);
  fastest_dct_block_mult(data + 6);

  /* - Sorting ------------------------------------------------------------- */

  tmp = data[1];
  data[1] = data[7];
  data[7] = data[4];
  data[4] = tmp;

  tmp = data[3];
  data[3] = data[5];
  data[5] = data[6];
  data[6] = tmp;
}


void fastest_dct_1d_y(fix_int_t *data)
{
  fix_int_t tmp;

  /* - Stage 1 ------------------------------------------------------------- */

  fastest_dct_block_butterfly(data + 0 * 8, data + 7 * 8);
  fastest_dct_block_butterfly(data + 1 * 8, data + 6 * 8);
  fastest_dct_block_butterfly(data + 2 * 8, data + 5 * 8);
  fastest_dct_block_butterfly(data + 3 * 8, data + 4 * 8);

  /* - Stage 2 ------------------------------------------------------------- */

  fastest_dct_block_butterfly(data + 0 * 8, data + 3 * 8);
  fastest_dct_block_butterfly(data + 1 * 8, data + 2 * 8);

  fastest_dct_block_rotation_1_3(data + 4 * 8, data + 7 * 8);
  fastest_dct_block_rotation_1_1(data + 5 * 8, data + 6 * 8);

  /* - Stage 3 ------------------------------------------------------------- */

  fastest_dct_block_butterfly(data, data + 1 * 8);
  fastest_dct_block_butterfly(data + 4 * 8, data + 6 * 8);
  fastest_dct_block_butterfly(data + 7 * 8, data + 5 * 8);
  fastest_dct_block_rotation_sqrt2_6(data + 2 * 8, data + 3 * 8);

  /* - Stage 4 ------------------------------------------------------------- */

  fastest_dct_block_butterfly(data + 7 * 8, data + 4 * 8);
  fastest_dct_block_mult(data + 5 * 8);
  fastest_dct_block_mult(data + 6 * 8);

  /* - Sorting ------------------------------------------------------------- */

  tmp = data[1 * 8];
  data[1 * 8] = data[7 * 8];
  data[7 * 8] = data[4 * 8];
  data[4 * 8] = tmp;

  tmp = data[3 * 8];
  data[3 * 8] = data[5 * 8];
  data[5 * 8] = data[6 * 8];
  data[6 * 8] = tmp;
}


void fastest_dct(int16_t *_data)
{
  fix_int_t data[64];

  for (int i = 0; i < 64; ++i)
    data[i] = FIX(_data[i]);

  for (int i = 0; i < 8; ++i)
    fastest_dct_1d_x(data + i * 8);

  for (int i = 0; i < 8; ++i)
    fastest_dct_1d_y(data + i);

  for (int i = 0; i < 64; ++i) {
    _data[i] = FIX_TO_INT(data[i]) / 8;
    if (_data[i] > 2047) _data[i] = 2047;
    if (_data[i] < -2047) _data[i] = -2047;
  }
}
