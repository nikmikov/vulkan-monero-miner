#include "crypto/skein256.h"

#include <stddef.h>

/* tweak word T[1]: flag bit definition(s) */
#define SKEIN_T1_BIT(BIT) ((BIT)-64)
#define SKEIN_T1_POS_FIRST SKEIN_T1_BIT(126) /* bits 126 : first block flag */
#define SKEIN_T1_POS_FINAL SKEIN_T1_BIT(127) /* bit  127 : final block flag */

/* tweak word T[1]: flag bit definition(s) */
#define SKEIN_T1_FLAG_FIRST (((uint64_t)1) << SKEIN_T1_POS_FIRST)
#define SKEIN_T1_FLAG_FINAL (((uint64_t)1) << SKEIN_T1_POS_FINAL)

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
void skein256_process_block(struct skein256_state *state, uint64_t *data)
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
  ts[0] += SKEIN_256_BLOCK_SIZE_BYTES;

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

void skein256_init(struct skein256_state *state)
{
  *state = (struct skein256_state){
      .chaining = {0xFC9DA860D048B449, 0x2FCA66479FA7D833, 0xB33BC3896656840F,
                   0x6A54E920FDE8DA69},
      .buffer = {0}};
}

void skein256_update(struct skein256_state *state, const void *data,
                     uint64_t databitlen)
{
}

void skein256_final(struct skein256_state *state, uint8_t *digest)
{
  /* tag as the final block */
  state->T[1] |= SKEIN_T1_FLAG_FINAL;
}

void skein256_hash(const void *in, uint64_t inlen_bytes, uint8_t *digest)
{
  struct skein256_state state;
  skein256_init(&state);
  skein256_update(&state, in, inlen_bytes * 8);
  skein256_final(&state, digest);
}
