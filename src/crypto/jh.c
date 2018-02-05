/* This program gives the optimized SSE2 bitslice implementation of JH for
 * 64-bit platform(with 16 128-bit XMM registers).
 *
 * Last Modified: January 16, 2011
 *
 * http://www3.ntu.edu.sg/home/wuhj/research/jh/
 *
 * Only 256 bit digest supported
 */
#include "crypto/jh.h"

#include <emmintrin.h>
#include <stdalign.h>
#include <string.h>

#include "crypto/jh_const.h"

#define CONSTANT(b) _mm_set1_epi8((char)(b))
#define XOR(x, y) _mm_xor_si128((x), (y))
#define AND(x, y) _mm_and_si128((x), (y))
#define OR(x, y) _mm_or_si128((x), (y))
#define ANDNOT(x, y) _mm_andnot_si128((x), (y))
#define LOAD(p) _mm_load_si128((__m128i *)(p))

#define SHR1(x) _mm_srli_epi16((x), 1)
#define SHR2(x) _mm_srli_epi16((x), 2)
#define SHR4(x) _mm_srli_epi16((x), 4)
#define SHR8(x) _mm_slli_epi16((x), 8)
#define SHR16(x) _mm_slli_epi32((x), 16)
#define SHR32(x) _mm_slli_epi64((x), 32)
#define SHR64(x) _mm_slli_si128((x), 8)
#define SHL1(x) _mm_slli_epi16((x), 1)
#define SHL2(x) _mm_slli_epi16((x), 2)
#define SHL4(x) _mm_slli_epi16((x), 4)
#define SHL8(x) _mm_srli_epi16((x), 8)
#define SHL16(x) _mm_srli_epi32((x), 16)
#define SHL32(x) _mm_srli_epi64((x), 32)
#define SHL64(x) _mm_srli_si128((x), 8)

/* swapping bit 2i with bit 2i+1 of the 128-bit x */
#define SWAP1(x)                                                               \
  OR(SHR1(AND((x), CONSTANT(0xaa))), SHL1(AND((x), CONSTANT(0x55))))

/* swapping bit 4i||4i+1 with bit 4i+2||4i+3 of the 128-bit x */
#define SWAP2(x)                                                               \
  OR(SHR2(AND((x), CONSTANT(0xcc))), SHL2(AND((x), CONSTANT(0x33))))

/* swapping bits 8i||8i+1||8i+2||8i+3 with  bits 8i+4||8i+5||8i+6||8i+7 of the
 * 128-bit x */
#define SWAP4(x)                                                               \
  OR(SHR4(AND((x), CONSTANT(0xf0))), SHL4(AND((x), CONSTANT(0xf))))

/* swapping bits 16i||16i+1||...||16i+7 with bits 16i+8||16i+9||...||16i+15 of
 * the 128-bit x */
#define SWAP8(x) OR(SHR8(x), SHL8(x))

/* swapping bits 32i||32i+1||...||32i+15 with bits 32i+16||32i+17||...||32i+31
 * of the 128-bit x */
#define SWAP16(x) OR(SHR16(x), SHL16(x))

/* swapping bits 64i||64i+1||...||64i+31 with bits 64i+32||64i+33||...||64i+63
 * of the 128-bit x */
#define SWAP32(x) _mm_shuffle_epi32((x), _MM_SHUFFLE(2, 3, 0, 1))

/* swapping bits 128i||128i+1||...||128i+63 with bits
 * 128i+64||128i+65||...||128i+127 of the 128-bit x */
#define SWAP64(x) _mm_shuffle_epi32((x), _MM_SHUFFLE(1, 0, 3, 2))

/* The MDS code */
#define L(m0, m1, m2, m3, m4, m5, m6, m7)                                      \
  (m4) = XOR((m4), (m1));                                                      \
  (m5) = XOR((m5), (m2));                                                      \
  (m6) = XOR(XOR((m6), (m3)), (m0));                                           \
  (m7) = XOR((m7), (m0));                                                      \
  (m0) = XOR((m0), (m5));                                                      \
  (m1) = XOR((m1), (m6));                                                      \
  (m2) = XOR(XOR((m2), (m7)), (m4));                                           \
  (m3) = XOR((m3), (m4));

/* Two Sboxes computed in parallel, each Sbox implements S0 and S1,
 * selected by a constant bit
 * The reason to compute two Sboxes in parallel is to try to fully utilize the
 * parallel processing power of SSE2 instructions */
#define SS(m0, m1, m2, m3, m4, m5, m6, m7, constant0, constant1)               \
  m3 = XOR(m3, CONSTANT(0xff));                                                \
  m7 = XOR(m7, CONSTANT(0xff));                                                \
  m0 = XOR(m0, ANDNOT(m2, constant0));                                         \
  m4 = XOR(m4, ANDNOT(m6, constant1));                                         \
  a0 = XOR(constant0, AND(m0, m1));                                            \
  a1 = XOR(constant1, AND(m4, m5));                                            \
  m0 = XOR(m0, AND(m3, m2));                                                   \
  m4 = XOR(m4, AND(m7, m6));                                                   \
  m3 = XOR(m3, ANDNOT(m1, m2));                                                \
  m7 = XOR(m7, ANDNOT(m5, m6));                                                \
  m1 = XOR(m1, AND(m0, m2));                                                   \
  m5 = XOR(m5, AND(m4, m6));                                                   \
  m2 = XOR(m2, ANDNOT(m3, m0));                                                \
  m6 = XOR(m6, ANDNOT(m7, m4));                                                \
  m0 = XOR(m0, OR(m1, m3));                                                    \
  m4 = XOR(m4, OR(m5, m7));                                                    \
  m3 = XOR(m3, AND(m1, m2));                                                   \
  m7 = XOR(m7, AND(m5, m6));                                                   \
  m2 = XOR(m2, a0);                                                            \
  m6 = XOR(m6, a1);                                                            \
  m1 = XOR(m1, AND(a0, m0));                                                   \
  m5 = XOR(m5, AND(a1, m4));

/* The linear transform of the (7*i+0)th round */
#define lineartransform_R00(m0, m1, m2, m3, m4, m5, m6, m7)                    \
  /* MDS layer */                                                              \
  L(m0, m1, m2, m3, m4, m5, m6, m7);                                           \
  /* swapping bit 2i with bit 2i+1 for m4,m5,m6 and m7 */                      \
  m4 = SWAP1(m4);                                                              \
  m5 = SWAP1(m5);                                                              \
  m6 = SWAP1(m6);                                                              \
  m7 = SWAP1(m7);

/* The linear transform of the (7*i+1)th round */
#define lineartransform_R01(m0, m1, m2, m3, m4, m5, m6, m7)                    \
  /* MDS layer */                                                              \
  L(m0, m1, m2, m3, m4, m5, m6, m7);                                           \
  /* swapping bit 4i||4i+1 with bit 4i+2||4i+3 for m4,m5,m6 and m7 */          \
  m4 = SWAP2(m4);                                                              \
  m5 = SWAP2(m5);                                                              \
  m6 = SWAP2(m6);                                                              \
  m7 = SWAP2(m7);

/* The linear transform of the (7*i+2)th round */
#define lineartransform_R02(m0, m1, m2, m3, m4, m5, m6, m7)                    \
  /*MDS layer*/                                                                \
  L(m0, m1, m2, m3, m4, m5, m6, m7);                                           \
  /* swapping bits 8i||8i+1||8i+2||8i+3 with bits 8i+4||8i+5||8i+6||8i+7 for   \
   * m4,m5,m6 and m7 */                                                        \
  m4 = SWAP4(m4);                                                              \
  m5 = SWAP4(m5);                                                              \
  m6 = SWAP4(m6);                                                              \
  m7 = SWAP4(m7);

/* The linear transform of the (7*i+3)th round */
#define lineartransform_R03(m0, m1, m2, m3, m4, m5, m6, m7)                    \
  /* MDS layer */                                                              \
  L(m0, m1, m2, m3, m4, m5, m6, m7);                                           \
  /* swapping bits 16i||16i+1||...||16i+7 with bits 16i+8||16i+9||...||16i+15  \
   * for m4,m5,m6 and m7 */                                                    \
  m4 = SWAP8(m4);                                                              \
  m5 = SWAP8(m5);                                                              \
  m6 = SWAP8(m6);                                                              \
  m7 = SWAP8(m7);

/* The linear transform of the (7*i+4)th round */
#define lineartransform_R04(m0, m1, m2, m3, m4, m5, m6, m7)                    \
  /* MDS layer */                                                              \
  L(m0, m1, m2, m3, m4, m5, m6, m7);                                           \
  /* swapping bits 32i||32i+1||...||32i+15 with bits                           \
   * 32i+16||32i+17||...||32i+31 for m0,m1,m2 and m3 */                        \
  m4 = SWAP16(m4);                                                             \
  m5 = SWAP16(m5);                                                             \
  m6 = SWAP16(m6);                                                             \
  m7 = SWAP16(m7);

/* The linear transform of the (7*i+5)th round -- faster */
#define lineartransform_R05(m0, m1, m2, m3, m4, m5, m6, m7)                    \
  /* MDS layer */                                                              \
  L(m0, m1, m2, m3, m4, m5, m6, m7);                                           \
  /* swapping bits 64i||64i+1||...||64i+31 with bits                           \
   * 64i+32||64i+33||...||64i+63 for m0,m1,m2 and m3 */                        \
  m4 = SWAP32(m4);                                                             \
  m5 = SWAP32(m5);                                                             \
  m6 = SWAP32(m6);                                                             \
  m7 = SWAP32(m7);

/* The linear transform of the (7*i+6)th round -- faster*/
#define lineartransform_R06(m0, m1, m2, m3, m4, m5, m6, m7)                    \
  /* MDS layer */                                                              \
  L(m0, m1, m2, m3, m4, m5, m6, m7);                                           \
  /* swapping bits 128i||128i+1||...||128i+63 with bits                        \
   * 128i+64||128i+65||...||128i+127 for m0,m1,m2 and m3 */                    \
  m4 = SWAP64(m4);                                                             \
  m5 = SWAP64(m5);                                                             \
  m6 = SWAP64(m6);                                                             \
  m7 = SWAP64(m7);

/* the round function of E8 */
#define round_function(nn, rr)                                                 \
  SS(y0, y2, y4, y6, y1, y3, y5, y7, JH256_E8_ROUNDCONSTANT_##rr##_EVN,        \
     JH256_E8_ROUNDCONSTANT_##rr##_ODD);                                       \
  lineartransform_R##nn(y0, y2, y4, y6, y1, y3, y5, y7);

/* The compression function F8 */
void jh_f8(struct jh_state *state)
{
  __m128i y0, y1, y2, y3, y4, y5, y6, y7;
  __m128i a0, a1;

  y0 = state->x0;
  y1 = state->x1;
  y2 = state->x2;
  y3 = state->x3;
  y4 = state->x4;
  y5 = state->x5;
  y6 = state->x6;
  y7 = state->x7;

  /*xor the 512-bit message with the fist half of the 1024-bit hash state*/
  y0 = XOR(y0, LOAD(state->buffer));
  y1 = XOR(y1, LOAD(state->buffer + 16));
  y2 = XOR(y2, LOAD(state->buffer + 32));
  y3 = XOR(y3, LOAD(state->buffer + 48));

  /*perform 42 rounds*/
  round_function(00, 00);
  round_function(01, 01);
  round_function(02, 02);
  round_function(03, 03);
  round_function(04, 04);
  round_function(05, 05);
  round_function(06, 06);

  round_function(00, 07);
  round_function(01, 08);
  round_function(02, 09);
  round_function(03, 10);
  round_function(04, 11);
  round_function(05, 12);
  round_function(06, 13);

  round_function(00, 14);
  round_function(01, 15);
  round_function(02, 16);
  round_function(03, 17);
  round_function(04, 18);
  round_function(05, 19);
  round_function(06, 20);

  round_function(00, 21);
  round_function(01, 22);
  round_function(02, 23);
  round_function(03, 24);
  round_function(04, 25);
  round_function(05, 26);
  round_function(06, 27);

  round_function(00, 28);
  round_function(01, 29);
  round_function(02, 30);
  round_function(03, 31);
  round_function(04, 32);
  round_function(05, 33);
  round_function(06, 34);

  round_function(00, 35);
  round_function(01, 36);
  round_function(02, 37);
  round_function(03, 38);
  round_function(04, 39);
  round_function(05, 40);
  round_function(06, 41);

  /*xor the 512-bit message with the second half of the 1024-bit hash state*/
  y4 = XOR(y4, LOAD(state->buffer));
  y5 = XOR(y5, LOAD(state->buffer + 16));
  y6 = XOR(y6, LOAD(state->buffer + 32));
  y7 = XOR(y7, LOAD(state->buffer + 48));

  state->x0 = y0;
  state->x1 = y1;
  state->x2 = y2;
  state->x3 = y3;
  state->x4 = y4;
  state->x5 = y5;
  state->x6 = y6;
  state->x7 = y7;
}

void jh_256_init(struct jh_state *state)
{
  state->databitlen = 0;
  state->datasize_in_buffer = 0;

  state->x0 = JH256_H0_0;
  state->x1 = JH256_H0_1;
  state->x2 = JH256_H0_2;
  state->x3 = JH256_H0_3;
  state->x4 = JH256_H0_4;
  state->x5 = JH256_H0_5;
  state->x6 = JH256_H0_6;
  state->x7 = JH256_H0_7;
}

/*hash each 512-bit message block, except the last partial block*/
void jh_update(struct jh_state *state, const void *data_ptr, size_t databitlen)
{
  const uint8_t *data = data_ptr;
  uint64_t index = 0;
  state->databitlen += databitlen;

  // if there is remaining data in the buffer,
  // fill it to a full message block first
  // we assume that the size of the data in the buffer is the multiple of 8 bits
  // if it is not at the end of a message

  // There is data in the buffer, but the incoming data is insufficient for a
  // full block
  if ((state->datasize_in_buffer > 0) &&
      ((state->datasize_in_buffer + databitlen) < 512)) {
    if ((databitlen & 7) == 0) {
      memcpy(state->buffer + (state->datasize_in_buffer >> 3), data,
             64 - (state->datasize_in_buffer >> 3));
    } else {
      memcpy(state->buffer + (state->datasize_in_buffer >> 3), data,
             64 - (state->datasize_in_buffer >> 3) + 1);
    }
    state->datasize_in_buffer += databitlen;
    databitlen = 0;
  }

  // There is data in the buffer, and the incoming data is sufficient for a full
  // block
  if ((state->datasize_in_buffer > 0) &&
      ((state->datasize_in_buffer + databitlen) >= 512)) {
    memcpy(state->buffer + (state->datasize_in_buffer >> 3), data,
           64 - (state->datasize_in_buffer >> 3));
    index = 64 - (state->datasize_in_buffer >> 3);
    databitlen = databitlen - (512 - state->datasize_in_buffer);
    jh_f8(state);
    state->datasize_in_buffer = 0;
  }

  // hash the remaining full message blocks
  for (; databitlen >= 512; index = index + 64, databitlen = databitlen - 512) {
    memcpy(state->buffer, data + index, 64);
    jh_f8(state);
  }

  // store the partial block into buffer, assume that -- if part of the last
  // byte is not part of the message, then that part consists of 0 bits
  if (databitlen > 0) {
    if ((databitlen & 7) == 0) {
      memcpy(state->buffer, data + index, (databitlen & 0x1ff) >> 3);
    } else {
      memcpy(state->buffer, data + index, ((databitlen & 0x1ff) >> 3) + 1);
    }
    state->datasize_in_buffer = databitlen;
  }
}

void jh_256_final(struct jh_state *state, uint8_t *digest)
{
  if ((state->databitlen & 0x1ff) == 0) {
    // pad the message when databitlen is multiple of 512 bits, then process the
    // padded block
    memset(state->buffer, 0, 64);
    state->buffer[0] = 0x80;
    state->buffer[63] = state->databitlen & 0xff;
    state->buffer[62] = (state->databitlen >> 8) & 0xff;
    state->buffer[61] = (state->databitlen >> 16) & 0xff;
    state->buffer[60] = (state->databitlen >> 24) & 0xff;
    state->buffer[59] = (state->databitlen >> 32) & 0xff;
    state->buffer[58] = (state->databitlen >> 40) & 0xff;
    state->buffer[57] = (state->databitlen >> 48) & 0xff;
    state->buffer[56] = (state->databitlen >> 56) & 0xff;
    jh_f8(state);
  } else {
    // set the rest of the bytes in the buffer to 0
    if ((state->datasize_in_buffer & 7) == 0) {
      for (size_t i = (state->databitlen & 0x1ff) >> 3; i < 64; i++) {
        state->buffer[i] = 0;
      }
    } else {
      for (size_t i = ((state->databitlen & 0x1ff) >> 3) + 1; i < 64; i++) {
        state->buffer[i] = 0;
      }
    }

    // pad and process the partial block when databitlen is not multiple of 512
    // bits, then hash the padded blocks
    state->buffer[((state->databitlen & 0x1ff) >> 3)] |=
        1 << (7 - (state->databitlen & 7));
    jh_f8(state);
    memset(state->buffer, 0, 64);
    state->buffer[63] = state->databitlen & 0xff;
    state->buffer[62] = (state->databitlen >> 8) & 0xff;
    state->buffer[61] = (state->databitlen >> 16) & 0xff;
    state->buffer[60] = (state->databitlen >> 24) & 0xff;
    state->buffer[59] = (state->databitlen >> 32) & 0xff;
    state->buffer[58] = (state->databitlen >> 40) & 0xff;
    state->buffer[57] = (state->databitlen >> 48) & 0xff;
    state->buffer[56] = (state->databitlen >> 56) & 0xff;
    jh_f8(state);
  }

  // truncating the final hash value to generate the message digest
  alignas(16) uint8_t t[32];
  _mm_store_si128((__m128i *)(t + 0), state->x6);
  _mm_store_si128((__m128i *)(t + 16), state->x7);

  memcpy(digest, t, 32);
}

void jh_256(const void *input, size_t inputbitlen, uint8_t *digest)
{
  struct jh_state state;
  jh_256_init(&state);
  jh_update(&state, input, inputbitlen);
  jh_256_final(&state, digest);
}
