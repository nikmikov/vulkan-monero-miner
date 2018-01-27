#include "crypto/skein.h"

#include "crypto/utils.h"
#include <string.h>

/* tweak word T[1]: flag bit definition(s) */
#define SKEIN_T1_BIT(BIT) ((BIT)-64)
#define SKEIN_T1_POS_FIRST SKEIN_T1_BIT(126) /* bits 126 : first block flag */
#define SKEIN_T1_POS_FINAL SKEIN_T1_BIT(127) /* bit 127 : final block flag */
#define SKEIN_T1_POS_BIT_PAD                                                   \
  SKEIN_T1_BIT(119) /* bit 119: partial final input byte */
#define SKEIN_T1_POS_BLK_TYPE SKEIN_T1_BIT(120) /* bits 120..125: type field   \
                                                 */

/* tweak word T[1]: flag bit definition(s) */
#define SKEIN_T1_FLAG_FIRST (((uint64_t)1) << SKEIN_T1_POS_FIRST)
#define SKEIN_T1_FLAG_FINAL (((uint64_t)1) << SKEIN_T1_POS_FINAL)
#define SKEIN_T1_FLAG_BIT_PAD (((uint64_t)1) << SKEIN_T1_POS_BIT_PAD)

#define SKEIN_T1_BLK_TYPE(T)                                                   \
  (((uint64_t)(SKEIN_BLK_TYPE_##T)) << SKEIN_T1_POS_BLK_TYPE)

/* message processing */
#define SKEIN_BLK_TYPE_MSG (48)
#define SKEIN_T1_BLK_TYPE_MSG SKEIN_T1_BLK_TYPE(MSG)
/* output stage */
#define SKEIN_BLK_TYPE_OUT (63)
#define SKEIN_T1_BLK_TYPE_OUT SKEIN_T1_BLK_TYPE(OUT)
#define SKEIN_T1_BLK_TYPE_OUT_FINAL                                            \
  (SKEIN_T1_BLK_TYPE_OUT | SKEIN_T1_FLAG_FINAL)

#define RotL_64(x, N) (((x) << (N)) | ((x) >> (64 - (N))))

enum {
  /* Skein_256 round rotation constants */
  R_512_0_0 = 46,
  R_512_0_1 = 36,
  R_512_0_2 = 19,
  R_512_0_3 = 37,
  R_512_1_0 = 33,
  R_512_1_1 = 27,
  R_512_1_2 = 14,
  R_512_1_3 = 42,
  R_512_2_0 = 17,
  R_512_2_1 = 49,
  R_512_2_2 = 36,
  R_512_2_3 = 39,
  R_512_3_0 = 44,
  R_512_3_1 = 9,
  R_512_3_2 = 54,
  R_512_3_3 = 56,
  R_512_4_0 = 39,
  R_512_4_1 = 30,
  R_512_4_2 = 34,
  R_512_4_3 = 24,
  R_512_5_0 = 13,
  R_512_5_1 = 50,
  R_512_5_2 = 10,
  R_512_5_3 = 17,
  R_512_6_0 = 25,
  R_512_6_1 = 29,
  R_512_6_2 = 39,
  R_512_6_3 = 43,
  R_512_7_0 = 8,
  R_512_7_1 = 35,
  R_512_7_2 = 56,
  R_512_7_3 = 22
};

/*  process single block  */
void skein_512_process_block(struct skein_512_state *state, uint64_t *data,
                             uint64_t byteCntAdd)
{
  static const size_t WCNT = SKEIN_512_STATE_WORDS;
  static const size_t RCNT = SKEIN_512_ROUNDS_TOTAL / 8;

  /* key schedule words : chaining vars + tweak + "rotation"*/
  uint64_t kw[WCNT + 4 + RCNT * 2];
  uint64_t *ks = kw + 3;
  uint64_t *ts = kw;

  /* local copy of context vars, for speed */
  uint64_t X0, X1, X2, X3, X4, X5, X6, X7;

  ts[0] = state->T[0] + byteCntAdd;
  ts[1] = state->T[1];

  /* this implementation only supports 2**64 input bytes (no carry out here) */

  /* precompute the key schedule for this block */
  ks[0] = state->chaining[0];
  ks[1] = state->chaining[1];
  ks[2] = state->chaining[2];
  ks[3] = state->chaining[3];
  ks[4] = state->chaining[4];
  ks[5] = state->chaining[5];
  ks[6] = state->chaining[6];
  ks[7] = state->chaining[7];
  ks[8] = ks[0] ^ ks[1] ^ ks[2] ^ ks[3] ^ ks[4] ^ ks[5] ^ ks[6] ^ ks[7] ^
          SKEIN_KS_PARITY;

  ts[2] = ts[0] ^ ts[1];

  /* do the first full key injection */
  X0 = data[0] + ks[0];
  X1 = data[1] + ks[1];
  X2 = data[2] + ks[2];
  X3 = data[3] + ks[3];
  X4 = data[4] + ks[4];
  X5 = data[5] + ks[5] + ts[0];
  X6 = data[6] + ks[6] + ts[1];
  X7 = data[7] + ks[7];

#define R512(p0, p1, p2, p3, p4, p5, p6, p7, ROT, rNum)                        \
  X##p0 += X##p1;                                                              \
  X##p1 = RotL_64(X##p1, ROT##_0);                                             \
  X##p1 ^= X##p0;                                                              \
  X##p2 += X##p3;                                                              \
  X##p3 = RotL_64(X##p3, ROT##_1);                                             \
  X##p3 ^= X##p2;                                                              \
  X##p4 += X##p5;                                                              \
  X##p5 = RotL_64(X##p5, ROT##_2);                                             \
  X##p5 ^= X##p4;                                                              \
  X##p6 += X##p7;                                                              \
  X##p7 = RotL_64(X##p7, ROT##_3);                                             \
  X##p7 ^= X##p6;

#define I512(R)                                                                \
  X0 += ks[((R) + 1) % 9]; /* inject the key schedule value */                 \
  X1 += ks[((R) + 2) % 9];                                                     \
  X2 += ks[((R) + 3) % 9];                                                     \
  X3 += ks[((R) + 4) % 9];                                                     \
  X4 += ks[((R) + 5) % 9];                                                     \
  X5 += ks[((R) + 6) % 9] + ts[((R) + 1) % 3];                                 \
  X6 += ks[((R) + 7) % 9] + ts[((R) + 2) % 3];                                 \
  X7 += ks[((R) + 8) % 9] + (R) + 1;

#define R512_8_rounds(R) /* do 8 full rounds */                                \
  R512(0, 1, 2, 3, 4, 5, 6, 7, R_512_0, 8 * (R) + 1);                          \
  R512(2, 1, 4, 7, 6, 5, 0, 3, R_512_1, 8 * (R) + 2);                          \
  R512(4, 1, 6, 3, 0, 5, 2, 7, R_512_2, 8 * (R) + 3);                          \
  R512(6, 1, 0, 7, 2, 5, 4, 3, R_512_3, 8 * (R) + 4);                          \
  I512(2 * (R));                                                               \
  R512(0, 1, 2, 3, 4, 5, 6, 7, R_512_4, 8 * (R) + 5);                          \
  R512(2, 1, 4, 7, 6, 5, 0, 3, R_512_5, 8 * (R) + 6);                          \
  R512(4, 1, 6, 3, 0, 5, 2, 7, R_512_6, 8 * (R) + 7);                          \
  R512(6, 1, 0, 7, 2, 5, 4, 3, R_512_7, 8 * (R) + 8);                          \
  I512(2 * (R) + 1); /* and key injection */

  // 72 rounds
  R512_8_rounds(0);
  R512_8_rounds(1);
  R512_8_rounds(2);
  R512_8_rounds(3);
  R512_8_rounds(4);
  R512_8_rounds(5);
  R512_8_rounds(6);
  R512_8_rounds(7);
  R512_8_rounds(8);

  /* do the final "feedforward" xor, update context chaining vars */
  state->chaining[0] = X0 ^ data[0];
  state->chaining[1] = X1 ^ data[1];
  state->chaining[2] = X2 ^ data[2];
  state->chaining[3] = X3 ^ data[3];
  state->chaining[4] = X4 ^ data[4];
  state->chaining[5] = X5 ^ data[5];
  state->chaining[6] = X6 ^ data[6];
  state->chaining[7] = X7 ^ data[7];

  ts[1] &= ~SKEIN_T1_FLAG_FIRST;

  state->T[0] = ts[0];
  state->T[1] = ts[1];
}

/* digest up to len bytes of input (full blocks only) */
static inline size_t skein_512_transform(struct skein_512_state *state,
                                         const uint8_t *const data,
                                         uint64_t len)
{
  /* increment block counter */
  /* digest message, one block at a time */
  const uint8_t *p = data;
  for (; len > SKEIN_512_BLOCK_SIZE;
       len -= SKEIN_512_BLOCK_SIZE, p += SKEIN_512_BLOCK_SIZE) {
    skein_512_process_block(state, (uint64_t *)data, SKEIN_512_BLOCK_SIZE);
  }
  return (size_t)(p - data);
}

void skein_512_256_init(struct skein_512_state *state)
{
  *state = (struct skein_512_state){
      .chaining = {0xCCD044A12FDB3E13, 0xE83590301A79A9EB, 0x55AEA0614F816E6F,
                   0x2A2767A4AE9B94DB, 0xEC06025E74DD7683, 0xE7A436CDC4746251,
                   0xC36FBAF9393AD185, 0x3EEDBA1833EDFC13},
      .T = {0, SKEIN_T1_FLAG_FIRST | SKEIN_T1_BLK_TYPE_MSG},
      .buffer = {0},
      .buf_ptr = 0,
      .bits_in_last_byte = 0};
  state->T[1] = SKEIN_T1_FLAG_FIRST | SKEIN_T1_BLK_TYPE_MSG;
}

void skein_512_update(struct skein_512_state *state, const void *dataptr,
                      size_t databitlen)
{
  size_t index = 0;
  size_t msglen = databitlen / 8;
  size_t rem = databitlen % 8;

  const uint8_t *data = dataptr;

  /* if the buffer contains data that has not yet been digested, first
     add data to buffer until full */
  if (state->buf_ptr) {
    while (state->buf_ptr < SKEIN_512_BLOCK_SIZE && index < msglen) {
      state->buffer[state->buf_ptr++] = data[index++];
    }
    if (state->buf_ptr < SKEIN_512_BLOCK_SIZE) {
      /* buffer still not full, return */
      if (rem) {
        state->bits_in_last_byte = rem;
        state->buffer[state->buf_ptr++] = data[index];
      }
      return;
    }

    /* digest buffer if there is more data to process */
    if (index < msglen) {
      state->buf_ptr = 0;
      skein_512_transform(state, state->buffer, SKEIN_512_BLOCK_SIZE);
    }
  }

  /* digest bulk of message */

  index += skein_512_transform(state, data + index, msglen - index);
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

void skein_512_final(struct skein_512_state *state, size_t digestbitlen,
                     uint8_t *digest)
{
  /* tag as the final block */
  state->T[1] |= SKEIN_T1_FLAG_FINAL;

  if (state->bits_in_last_byte) {
    /* handle partial final byte */
    const int BILB = state->bits_in_last_byte;
    /* partial byte bit mask */
    uint8_t mask = (uint8_t)(0x1 << (7 - BILB));
    /* apply bit padding on final byte */
    state->buffer[state->buf_ptr - 1] &= (0 - mask);
    state->buffer[state->buf_ptr - 1] |= mask;
  }

  if (state->buf_ptr > 0 || state->T[1] & SKEIN_T1_FLAG_FIRST) {
    size_t bytes_in_buf = state->buf_ptr;
    while (state->buf_ptr < SKEIN_512_BLOCK_SIZE) {
      state->buffer[state->buf_ptr++] = 0;
    }
    // process final block
    skein_512_process_block(state, (uint64_t *)state->buffer, bytes_in_buf);
  }
  /* now output the result */
  /* run Threefish in "counter mode" to generate output */
  /* zero out buffer[], so it can hold the counter */
  memset(state->buffer, 0, sizeof(state->buffer));

  /* build the counter block */
  state->T[0] = 0;
  state->T[1] = SKEIN_T1_FLAG_FIRST | SKEIN_T1_BLK_TYPE_OUT_FINAL;

  /* run "counter mode" */
  skein_512_process_block(state, (uint64_t *)state->buffer, sizeof(uint64_t));

  /* "output" the ctr mode bytes */
  memcpy(digest, state->chaining, digestbitlen / 8);
}

void skein_512_256(const void *input, size_t inputbitlen, uint8_t *digest)
{
  struct skein_512_state state;
  skein_512_256_init(&state);
  skein_512_update(&state, input, inputbitlen);
  skein_512_final(&state, 256, digest);
}
