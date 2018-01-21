/*
 * Based on AES-NI Groestl-256
 * Author: Krystian Matusiewicz, Günther A. Roland, Martin Schläffer
 *
 * This code is placed in the public domain
 */
#include "crypto/groestl256.h"

#include <assert.h>
#include <smmintrin.h>
#include <wmmintrin.h>

#include "crypto/groestl256_const.h"

#define GROESTL256_HASH_BIT_LEN 256
#define GROESTL256_COLS 8
#define GROESTL256_ROWS 8
#define GROESTL256_BLOCK_SIZE (GROESTL256_ROWS * GROESTL256_COLS)
#define GROESTL256_ROUNDS 10

struct groestl_state {
  uint64_t chaining[GROESTL256_BLOCK_SIZE / 8];
  uint8_t buffer[GROESTL256_BLOCK_SIZE];
  uint64_t block_counter;
  int buf_ptr;
  int bits_in_last_byte;
};

/* xmm[i] will be multiplied by 2
 * xmm[j] will be lost
 * xmm[k] has to be all 0x1b */
#define MUL2(i, j, k){\
  j = _mm_xor_si128(j, j);\
  j = _mm_cmpgt_epi8(j, i);\
  i = _mm_add_epi8(i, i);\
  j = _mm_and_si128(j, k);\
  i = _mm_xor_si128(i, j);\
}/**/

/* Matrix Transpose Step 1
 * input is a 512-bit state with two columns in one xmm
 * output is a 512-bit state with two rows in one xmm
 * inputs: i0-i3
 * outputs: i0, o1-o3
 * clobbers: t0
 */
#define Matrix_Transpose_A(i0, i1, i2, i3, o1, o2, o3, t0){ \
  t0 = GROESTL256_TRANSP_MASK;                              \
                                                            \
  i0 = _mm_shuffle_epi8(i0, t0);                            \
  i1 = _mm_shuffle_epi8(i1, t0);                            \
  i2 = _mm_shuffle_epi8(i2, t0);                            \
  i3 = _mm_shuffle_epi8(i3, t0);                            \
                                                            \
  o1 = i0;                                                  \
  t0 = i2;                                                  \
                                                            \
  i0 = _mm_unpacklo_epi16(i0, i1);                          \
  o1 = _mm_unpackhi_epi16(o1, i1);                          \
  i2 = _mm_unpacklo_epi16(i2, i3);                          \
  t0 = _mm_unpackhi_epi16(t0, i3);                          \
                                                            \
  i0 = _mm_shuffle_epi32(i0, 216);                          \
  o1 = _mm_shuffle_epi32(o1, 216);                          \
  i2 = _mm_shuffle_epi32(i2, 216);                          \
  t0 = _mm_shuffle_epi32(t0, 216);                          \
                                                            \
  o2 = i0;                                                  \
  o3 = o1;                                                  \
                                                            \
  i0 = _mm_unpacklo_epi32(i0, i2);                          \
  o1 = _mm_unpacklo_epi32(o1, t0);                          \
  o2 = _mm_unpackhi_epi32(o2, i2);                          \
  o3 = _mm_unpackhi_epi32(o3, t0);                          \
}/**/

/* Matrix Transpose Step 2
 * input are two 512-bit states with two rows in one xmm
 * output are two 512-bit states with one row of each state in one xmm
 * inputs: i0-i3 = P, i4-i7 = Q
 * outputs: (i0, o1-o7) = (P|Q)
 * possible reassignments: (output reg = input reg)
 * * i1 -> o3-7
 * * i2 -> o5-7
 * * i3 -> o7
 * * i4 -> o3-7
 * * i5 -> o6-7
 */
#define Matrix_Transpose_B(i0, i1, i2, i3, i4, i5, i6, i7, o1, o2, o3, o4, o5, o6, o7){\
  o1 = i0;\
  o2 = i1;\
  i0 = _mm_unpacklo_epi64(i0, i4);\
  o1 = _mm_unpackhi_epi64(o1, i4);\
  o3 = i1;\
  o4 = i2;\
  o2 = _mm_unpacklo_epi64(o2, i5);\
  o3 = _mm_unpackhi_epi64(o3, i5);\
  o5 = i2;\
  o6 = i3;\
  o4 = _mm_unpacklo_epi64(o4, i6);\
  o5 = _mm_unpackhi_epi64(o5, i6);\
  o7 = i3;\
  o6 = _mm_unpacklo_epi64(o6, i7);\
  o7 = _mm_unpackhi_epi64(o7, i7);\
}/**/


/* Matrix Transpose Inverse Step 2
 * input are two 512-bit states with one row of each state in one xmm
 * output are two 512-bit states with two rows in one xmm
 * inputs: i0-i7 = (P|Q)
 * outputs: (i0, i2, i4, i6) = P, (o0-o3) = Q
 */
#define Matrix_Transpose_B_INV(i0, i1, i2, i3, i4, i5, i6, i7, o0, o1, o2, o3){\
  o0 = i0;\
  i0 = _mm_unpacklo_epi64(i0, i1);\
  o0 = _mm_unpackhi_epi64(o0, i1);\
  o1 = i2;\
  i2 = _mm_unpacklo_epi64(i2, i3);\
  o1 = _mm_unpackhi_epi64(o1, i3);\
  o2 = i4;\
  i4 = _mm_unpacklo_epi64(i4, i5);\
  o2 = _mm_unpackhi_epi64(o2, i5);\
  o3 = i6;\
  i6 = _mm_unpacklo_epi64(i6, i7);\
  o3 = _mm_unpackhi_epi64(o3, i7);\
}/**/

/* Matrix Transpose Output Step 2
 * input is one 512-bit state with two rows in one xmm
 * output is one 512-bit state with one row in the low 64-bits of one xmm
 * inputs: i0,i2,i4,i6 = S
 * outputs: (i0-7) = (0|S)
 */
#define Matrix_Transpose_O_B(i0, i1, i2, i3, i4, i5, i6, i7, t0){\
  t0 = _mm_xor_si128(t0, t0);\
  i1 = i0;\
  i3 = i2;\
  i5 = i4;\
  i7 = i6;\
  i0 = _mm_unpacklo_epi64(i0, t0);\
  i1 = _mm_unpackhi_epi64(i1, t0);\
  i2 = _mm_unpacklo_epi64(i2, t0);\
  i3 = _mm_unpackhi_epi64(i3, t0);\
  i4 = _mm_unpacklo_epi64(i4, t0);\
  i5 = _mm_unpackhi_epi64(i5, t0);\
  i6 = _mm_unpacklo_epi64(i6, t0);\
  i7 = _mm_unpackhi_epi64(i7, t0);\
}/**/

/* Matrix Transpose Output Inverse Step 2
 * input is one 512-bit state with one row in the low 64-bits of one xmm
 * output is one 512-bit state with two rows in one xmm
 * inputs: i0-i7 = (0|S)
 * outputs: (i0, i2, i4, i6) = S
 */
#define Matrix_Transpose_O_B_INV(i0, i1, i2, i3, i4, i5, i6, i7){\
  i0 = _mm_unpacklo_epi64(i0, i1);\
  i2 = _mm_unpacklo_epi64(i2, i3);\
  i4 = _mm_unpacklo_epi64(i4, i5);\
  i6 = _mm_unpacklo_epi64(i6, i7);\
}/**/

/* Yet another implementation of MixBytes.
   This time we use the formulae (3) from the paper "Byte Slicing Groestl".
   Input: a0, ..., a7
   Output: b0, ..., b7 = MixBytes(a0,...,a7).
   but we use the relations:
   t_i = a_i + a_{i+3}
   x_i = t_i + t_{i+3}
   y_i = t_i + t+{i+2} + a_{i+6}
   z_i = 2*x_i
   w_i = z_i + y_{i+4}
   v_i = 2*w_i
   b_i = v_{i+3} + y_{i+4}
   We keep building b_i in registers xmm8..xmm15 by first building y_{i+4} there
   and then adding v_i computed in the meantime in registers xmm0..xmm7.
   We almost fit into 16 registers, need only 3 spills to memory.
   This implementation costs 7.7 c/b giving total speed on SNB: 10.7c/b.
   K. Matusiewicz, 2011/05/29 */
#define MixBytes(a0, a1, a2, a3, a4, a5, a6, a7, b0, b1, b2, b3, b4, b5, b6, b7){\
  /* t_i = a_i + a_{i+1} */\
  b6 = a0;\
  b7 = a1;\
  a0 = _mm_xor_si128(a0, a1);\
  b0 = a2;\
  a1 = _mm_xor_si128(a1, a2);\
  b1 = a3;\
  a2 = _mm_xor_si128(a2, a3);\
  b2 = a4;\
  a3 = _mm_xor_si128(a3, a4);\
  b3 = a5;\
  a4 = _mm_xor_si128(a4, a5);\
  b4 = a6;\
  a5 = _mm_xor_si128(a5, a6);\
  b5 = a7;\
  a6 = _mm_xor_si128(a6, a7);\
  a7 = _mm_xor_si128(a7, b6);\
  \
  /* build y4 y5 y6 ... in regs xmm8, xmm9, xmm10 by adding t_i*/\
  b0 = _mm_xor_si128(b0, a4);\
  b6 = _mm_xor_si128(b6, a4);\
  b1 = _mm_xor_si128(b1, a5);\
  b7 = _mm_xor_si128(b7, a5);\
  b2 = _mm_xor_si128(b2, a6);\
  b0 = _mm_xor_si128(b0, a6);\
  /* spill values y_4, y_5 to memory */\
  TEMP0 = b0;\
  b3 = _mm_xor_si128(b3, a7);\
  b1 = _mm_xor_si128(b1, a7);\
  TEMP1 = b1;\
  b4 = _mm_xor_si128(b4, a0);\
  b2 = _mm_xor_si128(b2, a0);\
  /* save values t0, t1, t2 to xmm8, xmm9 and memory */\
  b0 = a0;\
  b5 = _mm_xor_si128(b5, a1);\
  b3 = _mm_xor_si128(b3, a1);\
  b1 = a1;\
  b6 = _mm_xor_si128(b6, a2);\
  b4 = _mm_xor_si128(b4, a2);\
  TEMP2 = a2;\
  b7 = _mm_xor_si128(b7, a3);\
  b5 = _mm_xor_si128(b5, a3);\
  \
  /* compute x_i = t_i + t_{i+3} */\
  a0 = _mm_xor_si128(a0, a3);\
  a1 = _mm_xor_si128(a1, a4);\
  a2 = _mm_xor_si128(a2, a5);\
  a3 = _mm_xor_si128(a3, a6);\
  a4 = _mm_xor_si128(a4, a7);\
  a5 = _mm_xor_si128(a5, b0);\
  a6 = _mm_xor_si128(a6, b1);\
  a7 = _mm_xor_si128(a7, TEMP2);\
  \
  /* compute z_i : double x_i using temp xmm8 and 1B xmm9 */\
  /* compute w_i : add y_{i+4} */\
  b1 = GROESTL256_ALL_1B;\
  MUL2(a0, b0, b1);\
  a0 = _mm_xor_si128(a0, TEMP0);\
  MUL2(a1, b0, b1);\
  a1 = _mm_xor_si128(a1, TEMP1);\
  MUL2(a2, b0, b1);\
  a2 = _mm_xor_si128(a2, b2);\
  MUL2(a3, b0, b1);\
  a3 = _mm_xor_si128(a3, b3);\
  MUL2(a4, b0, b1);\
  a4 = _mm_xor_si128(a4, b4);\
  MUL2(a5, b0, b1);\
  a5 = _mm_xor_si128(a5, b5);\
  MUL2(a6, b0, b1);\
  a6 = _mm_xor_si128(a6, b6);\
  MUL2(a7, b0, b1);\
  a7 = _mm_xor_si128(a7, b7);\
  \
  /* compute v_i : double w_i      */\
  /* add to y_4 y_5 .. v3, v4, ... */\
  MUL2(a0, b0, b1);\
  b5 = _mm_xor_si128(b5, a0);\
  MUL2(a1, b0, b1);\
  b6 = _mm_xor_si128(b6, a1);\
  MUL2(a2, b0, b1);\
  b7 = _mm_xor_si128(b7, a2);\
  MUL2(a5, b0, b1);\
  b2 = _mm_xor_si128(b2, a5);\
  MUL2(a6, b0, b1);\
  b3 = _mm_xor_si128(b3, a6);\
  MUL2(a7, b0, b1);\
  b4 = _mm_xor_si128(b4, a7);\
  MUL2(a3, b0, b1);\
  MUL2(a4, b0, b1);\
  b0 = TEMP0;\
  b1 = TEMP1;\
  b0 = _mm_xor_si128(b0, a3);\
  b1 = _mm_xor_si128(b1, a4);\
}/*MixBytes*/

/* one round
 * i = round number
 * a0-a7 = input rows
 * b0-b7 = output rows
 */
#define ROUND(i, a0, a1, a2, a3, a4, a5, a6, a7, b0, b1, b2, b3, b4, b5, b6, b7){\
  /* AddRoundConstant */\
  b1 = GROESTL256_ROUND_CONST_Lx;\
  a0 = _mm_xor_si128(a0, (GROESTL256_ROUND_CONST_L0_##i));\
  a1 = _mm_xor_si128(a1, b1);\
  a2 = _mm_xor_si128(a2, b1);\
  a3 = _mm_xor_si128(a3, b1);\
  a4 = _mm_xor_si128(a4, b1);\
  a5 = _mm_xor_si128(a5, b1);\
  a6 = _mm_xor_si128(a6, b1);\
  a7 = _mm_xor_si128(a7, (GROESTL256_ROUND_CONST_L7_##i));\
  \
  /* ShiftBytes + SubBytes (interleaved) */\
  b0 = _mm_xor_si128(b0,  b0);\
  a0 = _mm_shuffle_epi8(a0, (GROESTL256_SUBSH_MASK_0));\
  a0 = _mm_aesenclast_si128(a0, b0);\
  a1 = _mm_shuffle_epi8(a1, (GROESTL256_SUBSH_MASK_1));\
  a1 = _mm_aesenclast_si128(a1, b0);\
  a2 = _mm_shuffle_epi8(a2, (GROESTL256_SUBSH_MASK_2));\
  a2 = _mm_aesenclast_si128(a2, b0);\
  a3 = _mm_shuffle_epi8(a3, (GROESTL256_SUBSH_MASK_3));\
  a3 = _mm_aesenclast_si128(a3, b0);\
  a4 = _mm_shuffle_epi8(a4, (GROESTL256_SUBSH_MASK_4));\
  a4 = _mm_aesenclast_si128(a4, b0);\
  a5 = _mm_shuffle_epi8(a5, (GROESTL256_SUBSH_MASK_5));\
  a5 = _mm_aesenclast_si128(a5, b0);\
  a6 = _mm_shuffle_epi8(a6, (GROESTL256_SUBSH_MASK_6));\
  a6 = _mm_aesenclast_si128(a6, b0);\
  a7 = _mm_shuffle_epi8(a7, (GROESTL256_SUBSH_MASK_7));\
  a7 = _mm_aesenclast_si128(a7, b0);\
  \
  /* MixBytes */\
  MixBytes(a0, a1, a2, a3, a4, a5, a6, a7, b0, b1, b2, b3, b4, b5, b6, b7);\
\
}


/* 10 rounds, P and Q in parallel */
#define ROUNDS_P_Q(){\
  ROUND(0, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,   \
           xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);        \
  ROUND(1, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8,   \
           xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15);        \
  ROUND(2, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,   \
           xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);        \
  ROUND(3, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8,   \
           xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15);        \
  ROUND(4, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,   \
           xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);        \
  ROUND(5, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8,   \
           xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15);        \
  ROUND(6, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,   \
           xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);        \
  ROUND(7, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8,   \
           xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15);        \
  ROUND(8, xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15,   \
           xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);        \
  ROUND(9, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7, xmm8,   \
           xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15);        \
}

static inline void groestl_tf512(uint64_t *h, uint64_t *m)
{
  __m128i* const chaining = (__m128i*) h;
  __m128i* const message = (__m128i*) m;
  static __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
  static __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
  static __m128i TEMP0;
  static __m128i TEMP1;
  static __m128i TEMP2;

  /* load message into registers xmm12 - xmm15 */
  xmm12 = message[0];
  xmm13 = message[1];
  xmm14 = message[2];
  xmm15 = message[3];

  /* transform message M from column ordering into row ordering */
  /* we first put two rows (64 bit) of the message into one 128-bit xmm register */
  Matrix_Transpose_A(xmm12, xmm13, xmm14, xmm15, xmm2, xmm6, xmm7, xmm0);

  /* load previous chaining value */
  /* we first put two rows (64 bit) of the CV into one 128-bit xmm register */
  xmm8 = chaining[0];
  xmm0 = chaining[1];
  xmm4 = chaining[2];
  xmm5 = chaining[3];

  /* xor message to CV get input of P */
  /* result: CV+M in xmm8, xmm0, xmm4, xmm5 */
  xmm8 = _mm_xor_si128(xmm8, xmm12);
  xmm0 = _mm_xor_si128(xmm0, xmm2);
  xmm4 = _mm_xor_si128(xmm4, xmm6);
  xmm5 = _mm_xor_si128(xmm5, xmm7);

  /* there are now 2 rows of the Groestl state (P and Q) in each xmm register */
  /* unpack to get 1 row of P (64 bit) and Q (64 bit) into one xmm register */
  /* result: the 8 rows of P and Q in xmm8 - xmm12 */
  Matrix_Transpose_B(xmm8, xmm0, xmm4, xmm5, xmm12, xmm2, xmm6, xmm7,
                     xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15);

  /* compute the two permutations P and Q in parallel */
  ROUNDS_P_Q();

  /* unpack again to get two rows of P or two rows of Q in one xmm register */
  Matrix_Transpose_B_INV(xmm8, xmm9, xmm10, xmm11, xmm12, xmm13,
                         xmm14, xmm15, xmm0, xmm1, xmm2, xmm3);

  /* xor output of P and Q */
  /* result: P(CV+M)+Q(M) in xmm0...xmm3 */
  xmm0 = _mm_xor_si128(xmm0, xmm8);
  xmm1 = _mm_xor_si128(xmm1, xmm10);
  xmm2 = _mm_xor_si128(xmm2, xmm12);
  xmm3 = _mm_xor_si128(xmm3, xmm14);

  /* xor CV (feed-forward) */
  /* result: P(CV+M)+Q(M)+CV in xmm0...xmm3 */
  xmm0 = _mm_xor_si128(xmm0, (chaining[0]));
  xmm1 = _mm_xor_si128(xmm1, (chaining[1]));
  xmm2 = _mm_xor_si128(xmm2, (chaining[2]));
  xmm3 = _mm_xor_si128(xmm3, (chaining[3]));

  /* store CV */
  chaining[0] = xmm0;
  chaining[1] = xmm1;
  chaining[2] = xmm2;
  chaining[3] = xmm3;
}

static inline void groestl256_output_transform(uint64_t *h)
{
  __m128i* const chaining = (__m128i*) h;
  static __m128i xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7;
  static __m128i xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15;
  static __m128i TEMP0;
  static __m128i TEMP1;
  static __m128i TEMP2;

  /* load CV into registers xmm8, xmm10, xmm12, xmm14 */
  xmm8 = chaining[0];
  xmm10 = chaining[1];
  xmm12 = chaining[2];
  xmm14 = chaining[3];

  /* there are now 2 rows of the CV in one xmm register */
  /* unpack to get 1 row of P (64 bit) into one half of an xmm register */
  /* result: the 8 input rows of P in xmm8 - xmm15 */
  Matrix_Transpose_O_B(xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15, xmm0);

  /* compute the permutation P */
  /* result: the output of P(CV) in xmm8 - xmm15 */
  ROUNDS_P_Q();

  /* unpack again to get two rows of P in one xmm register */
  /* result: P(CV) in xmm8, xmm10, xmm12, xmm14 */
  Matrix_Transpose_O_B_INV(xmm8, xmm9, xmm10, xmm11, xmm12, xmm13, xmm14, xmm15);

  /* xor CV to P output (feed-forward) */
  /* result: P(CV)+CV in xmm8, xmm10, xmm12, xmm14 */
  xmm8 = _mm_xor_si128(xmm8,  (chaining[0]));
  xmm10 = _mm_xor_si128(xmm10, (chaining[1]));
  xmm12 = _mm_xor_si128(xmm12, (chaining[2]));
  xmm14 = _mm_xor_si128(xmm14, (chaining[3]));

  /* transform state back from row ordering into column ordering */
  /* result: final hash value in xmm9, xmm11 */
  Matrix_Transpose_A(xmm8, xmm10, xmm12, xmm14, xmm4, xmm9, xmm11, xmm0);

  /* we only need to return the truncated half of the state */
  chaining[2] = xmm9;
  chaining[3] = xmm11;
}



/* digest up to len bytes of input (full blocks only) */
static inline void groestl256_transform(struct groestl_state *state,
                                        const uint8_t *data,
                                        uint64_t len)
{
  /* increment block counter */
  state->block_counter += len / GROESTL256_BLOCK_SIZE;

  /* digest message, one block at a time */
  for (;len >= GROESTL256_BLOCK_SIZE;len -= GROESTL256_BLOCK_SIZE, data += GROESTL256_BLOCK_SIZE) {
    groestl_tf512(state->chaining, (uint64_t*)data);
  }
  //asm volatile ("emms"); // TODO: check if it really needed
}

void groestl256_init(struct groestl_state *state)
{
  int i;
  for (i = 0; i <  GROESTL256_BLOCK_SIZE / 8; ++i) {
    state->chaining[i] = 0;
  }
  for (i = 0; i < GROESTL256_BLOCK_SIZE; ++i) {
    state->buffer[i] = 0;
  }
  /* set initial value */
  state->chaining[GROESTL256_COLS - 1] = 0xff00000000000000;
  __m128i* const chaining = (__m128i*)state->chaining;
  static __m128i xmm0, /*xmm1,*/ xmm2, /*xmm3, xmm4, xmm5,*/ xmm6, xmm7;
  static __m128i /*xmm8, xmm9, xmm10, xmm11,*/ xmm12, xmm13, xmm14, xmm15;

  /* load IV into registers xmm12 - xmm15 */
  xmm12 = chaining[0];
  xmm13 = chaining[1];
  xmm14 = chaining[2];
  xmm15 = chaining[3];

  /* transform chaining value from column ordering into row ordering */
  /* we put two rows (64 bit) of the IV into one 128-bit XMM register */
  Matrix_Transpose_A(xmm12, xmm13, xmm14, xmm15, xmm2, xmm6, xmm7, xmm0);

  /* store transposed IV */
  chaining[0] = xmm12;
  chaining[1] = xmm2;
  chaining[2] = xmm6;
  chaining[3] = xmm7;

  /* set other variables */
  state->buf_ptr = 0;
  state->block_counter = 0;
  state->bits_in_last_byte = 0;
}


void groestl256_final(struct groestl_state *state,
                      uint8_t *digest)
{
  int i, j = 0, hashbytelen = GROESTL256_HASH_BIT_LEN / 8;
  uint8_t *s = (uint8_t*)state->chaining;
  /* pad with '1'-bit and first few '0'-bits */
  if(state->bits_in_last_byte) {
    const int BILB = state->bits_in_last_byte;
    state->buffer[state->buf_ptr - 1] &= ((1<<BILB)-1)<<(8-BILB);
    state->buffer[state->buf_ptr - 1] ^= 0x1<<(7-BILB);
    state->bits_in_last_byte = 0;
  } else {
    state->buffer[state->buf_ptr++] = 0x80;
  }

  /* pad with '0'-bits */
  if (state->buf_ptr > GROESTL256_BLOCK_SIZE - GROESTL256_ROWS) {
    /* padding requires two blocks */
    while (state->buf_ptr < GROESTL256_BLOCK_SIZE) {
      state->buffer[state->buf_ptr++] = 0;
    }
    /* digest first padding block */
    groestl256_transform(state, state->buffer, GROESTL256_BLOCK_SIZE);
    state->buf_ptr = 0;
  }

  while (state->buf_ptr < GROESTL256_BLOCK_SIZE - GROESTL256_ROWS) {
    state->buffer[state->buf_ptr++] = 0;
  }

  /* length padding */
  state->block_counter++;
  state->buf_ptr = GROESTL256_BLOCK_SIZE;
  while (state->buf_ptr > GROESTL256_BLOCK_SIZE - GROESTL256_ROWS) {
    state->buffer[--state->buf_ptr] = (uint8_t)state->block_counter;
    state->block_counter >>= 8;
  }

  /* digest final padding block */
  groestl256_transform(state, state->buffer, GROESTL256_BLOCK_SIZE);
  /* perform output transformation */
  groestl256_output_transform(state->chaining);

  /* store hash result in output */
  for (i = GROESTL256_BLOCK_SIZE - hashbytelen; i < GROESTL256_BLOCK_SIZE; i++,j++) {
    digest[j] = s[i];
  }
}

static inline void groestl256_update(struct groestl_state *state,
                                     const uint8_t *data,
                                     uint64_t databitlen)
{
  int index = 0;
  int msglen = (int)(databitlen/8);
  int rem = (int)(databitlen%8);

  /* if the buffer contains data that has not yet been digested, first
     add data to buffer until full */
  if (state->buf_ptr) {
    while (state->buf_ptr < GROESTL256_BLOCK_SIZE && index < msglen) {
      state->buffer[state->buf_ptr++] = data[index++];
    }
    if (state->buf_ptr < GROESTL256_BLOCK_SIZE) {
      /* buffer still not full, return */
      if (rem) {
        state->bits_in_last_byte = rem;
        state->buffer[state->buf_ptr++] = data[index];
      }
      return;
    }

    /* digest buffer */
    state->buf_ptr = 0;
    groestl256_transform(state, state->buffer, GROESTL256_BLOCK_SIZE);
  }

  /* digest bulk of message */
  groestl256_transform(state, data + index, msglen-index);
  index += ((msglen - index) / GROESTL256_BLOCK_SIZE) * GROESTL256_BLOCK_SIZE;

  /* store remaining data in buffer */
  while (index < msglen) {
    state->buffer[state->buf_ptr++] = data[index++];
  }

  /* if non-integral number of bytes have been supplied, store
     remaining bits in last byte, together with information about
     number of bits */
  if (rem) {
    state->bits_in_last_byte = rem;
    state->buffer[state->buf_ptr++] = data[index];
  }
}

void groestl256_hash(uint8_t *out, const uint8_t *in, uint64_t databitlen)
{
  struct groestl_state state;
  groestl256_init(&state);
  groestl256_update(&state, in, databitlen);
  groestl256_final(&state, out);
}
