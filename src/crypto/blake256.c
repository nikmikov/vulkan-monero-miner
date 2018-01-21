/*
 * The blake256_* and blake224_* functions are largely copied from
 * blake256_light.c and blake224_light.c from the BLAKE website:
 *
 *     http://131002.net/blake/
 *
 * The hmac_* functions implement HMAC-BLAKE-256 and HMAC-BLAKE-224.
 * HMAC is specified by RFC 2104.
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "crypto/blake256.h"

#define U8TO32(p) \
    (((uint32_t)((p)[0]) << 24) | ((uint32_t)((p)[1]) << 16) |    \
     ((uint32_t)((p)[2]) <<  8) | ((uint32_t)((p)[3])      ))
#define U32TO8(p, v) \
    (p)[0] = (uint8_t)((v) >> 24); (p)[1] = (uint8_t)((v) >> 16); \
    (p)[2] = (uint8_t)((v) >>  8); (p)[3] = (uint8_t)((v)      );

const uint32_t cst[16] = {
  0x243F6A88, 0x85A308D3, 0x13198A2E, 0x03707344,
  0xA4093822, 0x299F31D0, 0x082EFA98, 0xEC4E6C89,
  0x452821E6, 0x38D01377, 0xBE5466CF, 0x34E90C6C,
  0xC0AC29B7, 0xC97C50DD, 0x3F84D5B5, 0xB5470917
};

const uint8_t sigma[][16] = {
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15},
    {14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3},
    {11, 8,12, 0, 5, 2,15,13,10,14, 3, 6, 7, 1, 9, 4},
    { 7, 9, 3, 1,13,12,11,14, 2, 6, 5,10, 4, 0,15, 8},
    { 9, 0, 5, 7, 2, 4,10,15,14, 1,11,12, 6, 8, 3,13},
    { 2,12, 6,10, 0,11, 8, 3, 4,13, 7, 5,15,14, 1, 9},
    {12, 5, 1,15,14,13, 4,10, 0, 7, 6, 3, 9, 2, 8,11},
    {13,11, 7,14,12, 1, 3, 9, 5, 0,15, 4, 8, 6, 2,10},
    { 6,15,14, 9,11, 3, 0, 8,12, 2,13, 7, 1, 4,10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5,15,11, 9,14, 3,12,13, 0},
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15},
    {14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3},
    {11, 8,12, 0, 5, 2,15,13,10,14, 3, 6, 7, 1, 9, 4},
    { 7, 9, 3, 1,13,12,11,14, 2, 6, 5,10, 4, 0,15, 8}
};

static const uint8_t padding[] = {
    0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

struct blake_state {
  uint32_t h[8], s[4], t[2];
  int buflen, nullt;
  uint8_t buf[64];
};

void blake256_compress(struct blake_state *state, const uint8_t *block) {
  uint32_t v[16], m[16], i;

#define ROT(x,n) (((x)<<(32-n))|((x)>>(n)))

#define G(a,b,c,d,e)                                    \
  v[a] += (m[sigma[i][e]] ^ cst[sigma[i][e+1]]) + v[b]; \
  v[d] = ROT(v[d] ^ v[a],16);                           \
  v[c] += v[d];                                         \
  v[b] = ROT(v[b] ^ v[c],12);                           \
  v[a] += (m[sigma[i][e+1]] ^ cst[sigma[i][e]])+v[b];   \
  v[d] = ROT(v[d] ^ v[a], 8);                           \
  v[c] += v[d];                                         \
  v[b] = ROT(v[b] ^ v[c], 7);

  for (i = 0; i < 16; ++i) {
    m[i] = U8TO32(block + i * 4);
  }
  for (i = 0; i < 8;  ++i) {
    v[i] = state->h[i];
  }

  v[ 8] = state->s[0] ^ 0x243F6A88;
  v[ 9] = state->s[1] ^ 0x85A308D3;
  v[10] = state->s[2] ^ 0x13198A2E;
  v[11] = state->s[3] ^ 0x03707344;
  v[12] = 0xA4093822;
  v[13] = 0x299F31D0;
  v[14] = 0x082EFA98;
  v[15] = 0xEC4E6C89;

  if (state->nullt == 0) {
    v[12] ^= state->t[0];
    v[13] ^= state->t[0];
    v[14] ^= state->t[1];
    v[15] ^= state->t[1];
  }

  for (i = 0; i < 14; ++i) {
    G(0, 4,  8, 12,  0);
    G(1, 5,  9, 13,  2);
    G(2, 6, 10, 14,  4);
    G(3, 7, 11, 15,  6);
    G(3, 4,  9, 14, 14);
    G(2, 7,  8, 13, 12);
    G(0, 5, 10, 15,  8);
    G(1, 6, 11, 12, 10);
  }

  for (i = 0; i < 16; ++i) {
    state->h[i % 8] ^= v[i];
  }
  for (i = 0; i < 8;  ++i) {
    state->h[i] ^= state->s[i % 4];
  }
}


static inline void blake256_init(struct blake_state *state) {
  state->h[0] = 0x6A09E667;
  state->h[1] = 0xBB67AE85;
  state->h[2] = 0x3C6EF372;
  state->h[3] = 0xA54FF53A;
  state->h[4] = 0x510E527F;
  state->h[5] = 0x9B05688C;
  state->h[6] = 0x1F83D9AB;
  state->h[7] = 0x5BE0CD19;
  state->t[0] = state->t[1] = state->buflen = state->nullt = 0;
  state->s[0] = state->s[1] = state->s[2] = state->s[3] = 0;
}

// datalen = number of bits
void blake256_update(struct blake_state *state, const uint8_t *data, uint64_t datalen) {
  int left = state->buflen >> 3;
  int fill = 64 - left;

  if (left && (((datalen >> 3) & 0x3F) >= (unsigned) fill)) {
    memcpy((void *) (state->buf + left), (void *) data, fill);
    state->t[0] += 512;
    if (state->t[0] == 0) state->t[1]++;
    blake256_compress(state, state->buf);
    data += fill;
    datalen -= (fill << 3);
    left = 0;
  }

  while (datalen >= 512) {
    state->t[0] += 512;
    if (state->t[0] == 0) state->t[1]++;
    blake256_compress(state, data);
    data += 64;
    datalen -= 512;
  }

  if (datalen > 0) {
    memcpy((void *) (state->buf + left), (void *) data, (size_t) (datalen >> 3));
    state->buflen = (left << 3) + (int) datalen;
  } else {
    state->buflen = 0;
  }
}

void blake256_final_h(struct blake_state *state, uint8_t *digest, uint8_t pa, uint8_t pb) {
  uint8_t msglen[8];
  uint32_t lo = state->t[0] + state->buflen, hi = state->t[1];
  if (lo < (unsigned) state->buflen) {
    ++hi;
  }
  U32TO8(msglen + 0, hi);
  U32TO8(msglen + 4, lo);

  if (state->buflen == 440) { /* one padding byte */
    state->t[0] -= 8;
    blake256_update(state, &pa, 8);
  } else {
    if (state->buflen < 440) { /* enough space to fill the block  */
      if (state->buflen == 0) {
        state->nullt = 1;
      }
      state->t[0] -= 440 - state->buflen;
      blake256_update(state, padding, 440 - state->buflen);
    } else { /* need 2 compressions */
      state->t[0] -= 512 - state->buflen;
      blake256_update(state, padding, 512 - state->buflen);
      state->t[0] -= 440;
      blake256_update(state, padding + 1, 440);
      state->nullt = 1;
    }
    blake256_update(state, &pb, 8);
    state->t[0] -= 8;
  }
  state->t[0] -= 64;
  blake256_update(state, msglen, 64);

  U32TO8(digest +  0, state->h[0]);
  U32TO8(digest +  4, state->h[1]);
  U32TO8(digest +  8, state->h[2]);
  U32TO8(digest + 12, state->h[3]);
  U32TO8(digest + 16, state->h[4]);
  U32TO8(digest + 20, state->h[5]);
  U32TO8(digest + 24, state->h[6]);
  U32TO8(digest + 28, state->h[7]);
}

void blake256_final(struct blake_state *state, uint8_t *digest) {
  blake256_final_h(state, digest, 0x81, 0x01);
}

// inlen = number of bytes
void blake256_hash(uint8_t *out, const uint8_t *in, uint64_t inlen) {
  struct blake_state state;
  blake256_init(&state);
  blake256_update(&state, in, inlen * 8);
  blake256_final(&state, out);
}
