/* This program gives the optimized SSE2 bitslice implementation of JH for
 * 64-bit platform(with 16 128-bit XMM registers).
 *
 * Last Modified: January 16, 2011
 *
 * http://www3.ntu.edu.sg/home/wuhj/research/jh/
 *
 * ^ Only 256 bit digest supported
 */
#pragma once

#include <stdint.h>

#include <xmmintrin.h>

struct jh256_state {
  /*the message size in bits*/
  uint64_t databitlen;

  /* the size of the message remained in buffer; assumed to be multiple of 8bits
   * except for the last partial block at the end of the message */
  uint64_t datasize_in_buffer;

  /*1024-bit state;*/
  __m128i x0, x1, x2, x3, x4, x5, x6, x7;

  /* 512-bit message block; */
  uint8_t buffer[64];
};

void jh256_init(struct jh256_state *state);
void jh256_update(struct jh256_state *state, const void *data,
                  uint64_t databitlen);
void jh256_final(struct jh256_state *state, uint8_t *digest);

void jh256_hash(const void *in, uint64_t inlen_bytes, uint8_t *digest);
