#include "crypto/skein.h"

#include <string.h>

/* tweak word T[1]: flag bit definition(s) */
#define SKEIN_T1_BIT(BIT)      ((BIT)-64)
#define SKEIN_T1_POS_FIRST     SKEIN_T1_BIT(126) /* bits 126 : first block flag */
#define SKEIN_T1_POS_FINAL     SKEIN_T1_BIT(127) /* bit 127 : final block flag */
#define SKEIN_T1_POS_BIT_PAD   SKEIN_T1_BIT(119) /* bit 119 : partial final input byte */
#define SKEIN_T1_POS_BLK_TYPE  SKEIN_T1_BIT(120) /* bits 120..125: type field */

/* tweak word T[1]: flag bit definition(s) */
#define SKEIN_T1_FLAG_FIRST (((uint64_t)1) << SKEIN_T1_POS_FIRST)
#define SKEIN_T1_FLAG_FINAL (((uint64_t)1) << SKEIN_T1_POS_FINAL)
#define SKEIN_T1_FLAG_BIT_PAD (((uint64_t)1) << SKEIN_T1_POS_BIT_PAD)

#define SKEIN_T1_BLK_TYPE(T) (((uint64_t) (SKEIN_BLK_TYPE_##T)) << SKEIN_T1_POS_BLK_TYPE)

/* message processing */
#define SKEIN_BLK_TYPE_MSG      (48)
#define SKEIN_T1_BLK_TYPE_MSG   SKEIN_T1_BLK_TYPE(MSG)
 /* output stage */
#define SKEIN_BLK_TYPE_OUT      (63)
#define SKEIN_T1_BLK_TYPE_OUT   SKEIN_T1_BLK_TYPE(OUT)
#define SKEIN_T1_BLK_TYPE_OUT_FINAL (SKEIN_T1_BLK_TYPE_OUT | SKEIN_T1_FLAG_FINAL)

#define RotL_64(x, N) (((x) << (N)) | ((x) >> (64 - (N))))

enum {
  /* Skein_256 round rotation constants */
  R_256_0_0 = 5,
  R_256_0_1 = 56,
  R_256_1_0 = 36,
  R_256_1_1 = 28,
  R_256_2_0 = 13,
  R_256_2_1 = 46,
  R_256_3_0 = 58,
  R_256_3_1 = 44,
  R_256_4_0 = 26,
  R_256_4_1 = 20,
  R_256_5_0 = 53,
  R_256_5_1 = 35,
  R_256_6_0 = 11,
  R_256_6_1 = 42,
  R_256_7_0 = 59,
  R_256_7_1 = 50
};

/*  process single block  */
void skein_256_process_block(struct skein_256_state *state, uint64_t *data)
{
  static const size_t WCNT = SKEIN_256_STATE_WORDS;
  static const size_t RCNT = SKEIN_256_ROUNDS_TOTAL / 8;

  /* key schedule words : chaining vars + tweak + "rotation"*/
  uint64_t kw[WCNT + 4 + RCNT * 2];
  uint64_t *ks = kw + 3;
  uint64_t *ts = kw;

  /* local copy of context vars, for speed */
  uint64_t X0, X1, X2, X3;

  ts[0] = state->T[0];
  ts[1] = state->T[1];

  /* this implementation only supports 2**64 input bytes (no carry out here) */

  /* update processed length */
  ts[0] += SKEIN_256_BLOCK_SIZE;

  /* precompute the key schedule for this block */
  ks[0] = state->chaining[0];
  ks[1] = state->chaining[1];
  ks[2] = state->chaining[2];
  ks[3] = state->chaining[3];
  ks[4] = ks[0] ^ ks[1] ^ ks[2] ^ ks[3] ^ SKEIN_KS_PARITY;

  ts[2] = ts[0] ^ ts[1];

  /* do the first full key injection */
  X0 = data[0] + ks[0];
  X1 = data[1] + ks[1] + ts[0];
  X2 = data[2] + ks[2] + ts[1];
  X3 = data[3] + ks[3];

#define R256(p0, p1, p2, p3, ROT, rNum)                                        \
  X##p0 += X##p1;                                                              \
  X##p1 = RotL_64(X##p1, ROT##_0);                                             \
  X##p1 ^= X##p0;                                                              \
  X##p2 += X##p3;                                                              \
  X##p3 = RotL_64(X##p3, ROT##_1);                                             \
  X##p3 ^= X##p2;

#define I256(R)                                                                \
  X0 += ks[r + (R) + 0]; /* inject the key schedule value */                   \
  X1 += ks[r + (R) + 1] + ts[r + (R) + 0];                                     \
  X2 += ks[r + (R) + 2] + ts[r + (R) + 1];                                     \
  X3 += ks[r + (R) + 3] + r + (R);                                             \
  ks[r + (R) + 4] = ks[r + (R)-1]; /* rotate key schedule */                   \
  ts[r + (R) + 2] = ts[r + (R)-1];

#define R256_8_rounds(R)                                                       \
  R256(0, 1, 2, 3, R_256_0, 8 * (R) + 1);                                      \
  R256(0, 3, 2, 1, R_256_1, 8 * (R) + 2);                                      \
  R256(0, 1, 2, 3, R_256_2, 8 * (R) + 3);                                      \
  R256(0, 3, 2, 1, R_256_3, 8 * (R) + 4);                                      \
  I256(2 * (R));                                                               \
  R256(0, 1, 2, 3, R_256_4, 8 * (R) + 5);                                      \
  R256(0, 3, 2, 1, R_256_5, 8 * (R) + 6);                                      \
  R256(0, 1, 2, 3, R_256_6, 8 * (R) + 7);                                      \
  R256(0, 3, 2, 1, R_256_7, 8 * (R) + 8);                                      \
  I256(2 * (R) + 1);

  static const size_t r = 1;
  // 72 rounds
  R256_8_rounds(0);
  R256_8_rounds(1);
  R256_8_rounds(2);
  R256_8_rounds(3);
  R256_8_rounds(4);
  R256_8_rounds(5);
  R256_8_rounds(6);
  R256_8_rounds(7);
  R256_8_rounds(8);

  /* do the final "feedforward" xor, update context chaining vars */
  state->chaining[0] = X0 ^ data[0];
  state->chaining[1] = X1 ^ data[1];
  state->chaining[2] = X2 ^ data[2];
  state->chaining[3] = X3 ^ data[3];

  ts[1] &= ~SKEIN_T1_FLAG_FIRST;

  state->T[0] = ts[0];
  state->T[1] = ts[1];
}

/* digest up to len bytes of input (full blocks only) */
static inline void skein_256_transform(struct skein_256_state *state,
                                       const uint8_t *data, uint64_t len)
{
  /* increment block counter */
  state->block_counter += len / SKEIN_256_BLOCK_SIZE;

  /* digest message, one block at a time */
  for (; len >= SKEIN_256_BLOCK_SIZE;
       len -= SKEIN_256_BLOCK_SIZE, data += SKEIN_256_BLOCK_SIZE) {
    skein_256_process_block(state, (uint64_t*)data);
  }
}

void skein_256_init(struct skein_256_state *state)
{
  *state = (struct skein_256_state){
      .chaining = {0xFC9DA860D048B449, 0x2FCA66479FA7D833, 0xB33BC3896656840F,
                   0x6A54E920FDE8DA69},
      .T = {0, SKEIN_T1_FLAG_FIRST | SKEIN_T1_BLK_TYPE_MSG},
      .buffer = {0},
      .block_counter = 0,
      .buf_ptr = 0,
      .bits_in_last_byte = 0};
}

void skein_256_update(struct skein_256_state *state, const void *dataptr,
                      size_t databitlen)
{
  size_t index = 0;
  size_t msglen = databitlen / 8;
  size_t rem = databitlen % 8;

  const uint8_t *data = dataptr;

  /* if the buffer contains data that has not yet been digested, first
     add data to buffer until full */
  if (state->buf_ptr) {
    while (state->buf_ptr < SKEIN_256_BLOCK_SIZE && index < msglen) {
      state->buffer[state->buf_ptr++] = data[index++];
    }
    if (state->buf_ptr < SKEIN_256_BLOCK_SIZE) {
      /* buffer still not full, return */
      if (rem) {
        state->bits_in_last_byte = rem;
        state->buffer[state->buf_ptr++] = data[index];
      }
      return;
    }

    /* digest buffer */
    state->buf_ptr = 0;
    skein_256_transform(state, state->buffer, SKEIN_256_BLOCK_SIZE);
  }

  /* digest bulk of message */
  skein_256_transform(state, data + index, msglen - index);
  index += ((msglen - index) / SKEIN_256_BLOCK_SIZE) * SKEIN_256_BLOCK_SIZE;
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

void skein_256_final(struct skein_256_state *state, uint8_t *digest)
{
  /* tag as the final block */
  state->T[1] |= SKEIN_T1_FLAG_FINAL;

  if (state->bits_in_last_byte) {
    /* handle partial final byte */
    const int BILB = state->bits_in_last_byte;
     /* partial byte bit mask */
    uint8_t mask = (uint8_t) (0x1 << (7 - BILB));
    /* apply bit padding on final byte */
    state->buffer[state->buf_ptr - 1] &= (0 - mask);
    state->buffer[state->buf_ptr - 1] |= mask;
  }

  if(state->buf_ptr > 0) {
    while(state->buf_ptr < SKEIN_256_BLOCK_SIZE) {
      state->buffer[state->buf_ptr++] = 0;
    }
    // process final block
    skein_256_process_block(state, (uint64_t*)state->buffer);
  }

  /* now output the result */
  /* run Threefish in "counter mode" to generate output */
  /* zero out buffer[], so it can hold the counter */
  memset(state->buffer,0, sizeof(state->buffer));

  /* build the counter block */
  state->T[0] = 0;
  state->T[1] = SKEIN_T1_BLK_TYPE_OUT_FINAL;

  /* run "counter mode" */
  skein_256_process_block(state, (uint64_t*)state->buffer);

  /* "output" the ctr mode bytes */
  memcpy(digest, state->chaining, SKEIN_256_HASH_BIT_LEN / 8);

}

void skein_256(const void *input, size_t inputbitlen, uint8_t *digest)
{
  struct skein_256_state state;
  skein_256_init(&state);
  skein_256_update(&state, input, inputbitlen);
  skein_256_final(&state, digest);
}
