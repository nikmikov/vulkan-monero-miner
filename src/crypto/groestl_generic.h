#pragma once

#include "crypto/groestl_generic_const.h"

#define ROTATE_COLUMN_DOWN(v1, v2, amount_bytes, temp_var)                     \
  {                                                                            \
    temp_var = (v1 << (8 * amount_bytes)) | (v2 >> (8 * (4 - amount_bytes)));  \
    v2 = (v2 << (8 * amount_bytes)) | (v1 >> (8 * (4 - amount_bytes)));        \
    v1 = temp_var;                                                             \
  }

#define COLUMN(x, y, i, c0, c1, c2, c3, c4, c5, c6, c7, tv1, tv2, tu, tl, t)   \
  tu = T[2 * (uint32_t)x[4 * c0 + 0]];                                         \
  tl = T[2 * (uint32_t)x[4 * c0 + 0] + 1];                                     \
  tv1 = T[2 * (uint32_t)x[4 * c1 + 1]];                                        \
  tv2 = T[2 * (uint32_t)x[4 * c1 + 1] + 1];                                    \
  ROTATE_COLUMN_DOWN(tv1, tv2, 1, t)                                           \
  tu ^= tv1;                                                                   \
  tl ^= tv2;                                                                   \
  tv1 = T[2 * (uint32_t)x[4 * c2 + 2]];                                        \
  tv2 = T[2 * (uint32_t)x[4 * c2 + 2] + 1];                                    \
  ROTATE_COLUMN_DOWN(tv1, tv2, 2, t)                                           \
  tu ^= tv1;                                                                   \
  tl ^= tv2;                                                                   \
  tv1 = T[2 * (uint32_t)x[4 * c3 + 3]];                                        \
  tv2 = T[2 * (uint32_t)x[4 * c3 + 3] + 1];                                    \
  ROTATE_COLUMN_DOWN(tv1, tv2, 3, t)                                           \
  tu ^= tv1;                                                                   \
  tl ^= tv2;                                                                   \
  tl ^= T[2 * (uint32_t)x[4 * c4 + 0]];                                        \
  tu ^= T[2 * (uint32_t)x[4 * c4 + 0] + 1];                                    \
  tv1 = T[2 * (uint32_t)x[4 * c5 + 1]];                                        \
  tv2 = T[2 * (uint32_t)x[4 * c5 + 1] + 1];                                    \
  ROTATE_COLUMN_DOWN(tv1, tv2, 1, t)                                           \
  tl ^= tv1;                                                                   \
  tu ^= tv2;                                                                   \
  tv1 = T[2 * (uint32_t)x[4 * c6 + 2]];                                        \
  tv2 = T[2 * (uint32_t)x[4 * c6 + 2] + 1];                                    \
  ROTATE_COLUMN_DOWN(tv1, tv2, 2, t)                                           \
  tl ^= tv1;                                                                   \
  tu ^= tv2;                                                                   \
  tv1 = T[2 * (uint32_t)x[4 * c7 + 3]];                                        \
  tv2 = T[2 * (uint32_t)x[4 * c7 + 3] + 1];                                    \
  ROTATE_COLUMN_DOWN(tv1, tv2, 3, t)                                           \
  tl ^= tv1;                                                                   \
  tu ^= tv2;                                                                   \
  y[i] = tu;                                                                   \
  y[i + 1] = tl;

/* compute one round of P (short variants) */
static inline void RND512P(uint8_t *x, uint32_t *y, uint32_t r)
{
  uint32_t temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp;
  uint32_t *x32 = (uint32_t *)x;
  x32[0] ^= 0x00000000 ^ r;
  x32[2] ^= 0x00000010 ^ r;
  x32[4] ^= 0x00000020 ^ r;
  x32[6] ^= 0x00000030 ^ r;
  x32[8] ^= 0x00000040 ^ r;
  x32[10] ^= 0x00000050 ^ r;
  x32[12] ^= 0x00000060 ^ r;
  x32[14] ^= 0x00000070 ^ r;
  COLUMN(x, y, 0, 0, 2, 4, 6, 9, 11, 13, 15, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 2, 2, 4, 6, 8, 11, 13, 15, 1, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 4, 4, 6, 8, 10, 13, 15, 1, 3, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 6, 6, 8, 10, 12, 15, 1, 3, 5, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 8, 8, 10, 12, 14, 1, 3, 5, 7, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 10, 10, 12, 14, 0, 3, 5, 7, 9, temp_v1, temp_v2,
         temp_upper_value, temp_lower_value, temp);
  COLUMN(x, y, 12, 12, 14, 0, 2, 5, 7, 9, 11, temp_v1, temp_v2,
         temp_upper_value, temp_lower_value, temp);
  COLUMN(x, y, 14, 14, 0, 2, 4, 7, 9, 11, 13, temp_v1, temp_v2,
         temp_upper_value, temp_lower_value, temp);
}

/* compute one round of Q (short variants) */
static inline void RND512Q(uint8_t *x, uint32_t *y, uint32_t r)
{
  uint32_t temp_v1, temp_v2, temp_upper_value, temp_lower_value, temp;
  uint32_t *x32 = (uint32_t *)x;
  x32[0] = ~x32[0];
  x32[1] ^= 0xffffffff ^ r;
  x32[2] = ~x32[2];
  x32[3] ^= 0xefffffff ^ r;
  x32[4] = ~x32[4];
  x32[5] ^= 0xdfffffff ^ r;
  x32[6] = ~x32[6];
  x32[7] ^= 0xcfffffff ^ r;
  x32[8] = ~x32[8];
  x32[9] ^= 0xbfffffff ^ r;
  x32[10] = ~x32[10];
  x32[11] ^= 0xafffffff ^ r;
  x32[12] = ~x32[12];
  x32[13] ^= 0x9fffffff ^ r;
  x32[14] = ~x32[14];
  x32[15] ^= 0x8fffffff ^ r;
  COLUMN(x, y, 0, 2, 6, 10, 14, 1, 5, 9, 13, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 2, 4, 8, 12, 0, 3, 7, 11, 15, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 4, 6, 10, 14, 2, 5, 9, 13, 1, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 6, 8, 12, 0, 4, 7, 11, 15, 3, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 8, 10, 14, 2, 6, 9, 13, 1, 5, temp_v1, temp_v2, temp_upper_value,
         temp_lower_value, temp);
  COLUMN(x, y, 10, 12, 0, 4, 8, 11, 15, 3, 7, temp_v1, temp_v2,
         temp_upper_value, temp_lower_value, temp);
  COLUMN(x, y, 12, 14, 2, 6, 10, 13, 1, 5, 9, temp_v1, temp_v2,
         temp_upper_value, temp_lower_value, temp);
  COLUMN(x, y, 14, 0, 4, 8, 12, 15, 3, 7, 11, temp_v1, temp_v2,
         temp_upper_value, temp_lower_value, temp);
}

/* compute compression function (short variants) */
static inline void groestl_tf512(uint64_t *hash, const uint8_t *memory)
{
  uint32_t *h = (uint32_t *)hash;
  uint32_t *m = (uint32_t *)memory;
  int i;
  uint32_t Ptmp[2 * GROESTL256_COLS];
  uint32_t Qtmp[2 * GROESTL256_COLS];
  uint32_t y[2 * GROESTL256_COLS];
  uint32_t z[2 * GROESTL256_COLS];

  for (i = 0; i < 2 * GROESTL256_COLS; ++i) {
    z[i] = m[i];
    Ptmp[i] = h[i] ^ m[i];
  }

  /* compute Q(m) */
  RND512Q((uint8_t *)z, y, 0x00000000);
  RND512Q((uint8_t *)y, z, 0x01000000);
  RND512Q((uint8_t *)z, y, 0x02000000);
  RND512Q((uint8_t *)y, z, 0x03000000);
  RND512Q((uint8_t *)z, y, 0x04000000);
  RND512Q((uint8_t *)y, z, 0x05000000);
  RND512Q((uint8_t *)z, y, 0x06000000);
  RND512Q((uint8_t *)y, z, 0x07000000);
  RND512Q((uint8_t *)z, y, 0x08000000);
  RND512Q((uint8_t *)y, Qtmp, 0x09000000);

  /* compute P(h+m) */
  RND512P((uint8_t *)Ptmp, y, 0x00000000);
  RND512P((uint8_t *)y, z, 0x00000001);
  RND512P((uint8_t *)z, y, 0x00000002);
  RND512P((uint8_t *)y, z, 0x00000003);
  RND512P((uint8_t *)z, y, 0x00000004);
  RND512P((uint8_t *)y, z, 0x00000005);
  RND512P((uint8_t *)z, y, 0x00000006);
  RND512P((uint8_t *)y, z, 0x00000007);
  RND512P((uint8_t *)z, y, 0x00000008);
  RND512P((uint8_t *)y, Ptmp, 0x00000009);

  /* compute P(h+m) + Q(m) + h */
  for (i = 0; i < 2 * GROESTL256_COLS; ++i) {
    h[i] ^= Ptmp[i] ^ Qtmp[i];
  }
}

/* given state h, do h <- P(h)+h */
static inline void groestl_256_output_transform(uint64_t *h)
{

  uint32_t *chaining = (uint32_t *)h;

  int j;
  uint32_t temp[2 * GROESTL256_COLS];
  uint32_t y[2 * GROESTL256_COLS];
  uint32_t z[2 * GROESTL256_COLS];

  for (j = 0; j < 2 * GROESTL256_COLS; ++j) {
    temp[j] = chaining[j];
  }
  RND512P((uint8_t *)temp, y, 0x00000000);
  RND512P((uint8_t *)y, z, 0x00000001);
  RND512P((uint8_t *)z, y, 0x00000002);
  RND512P((uint8_t *)y, z, 0x00000003);
  RND512P((uint8_t *)z, y, 0x00000004);
  RND512P((uint8_t *)y, z, 0x00000005);
  RND512P((uint8_t *)z, y, 0x00000006);
  RND512P((uint8_t *)y, z, 0x00000007);
  RND512P((uint8_t *)z, y, 0x00000008);
  RND512P((uint8_t *)y, temp, 0x00000009);
  for (j = 0; j < 2 * GROESTL256_COLS; ++j) {
    chaining[j] ^= temp[j];
  }
}
