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
#include <stddef.h>

#include <xmmintrin.h>

struct jh_state {
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

/** stream processing */
void jh_256_init(struct jh_state *state);
void jh_update(struct jh_state *state, const void *data,
               size_t databitlen);
void jh_256_final(struct jh_state *state, uint8_t *digest);

/** hash fixed size input and produce 256-bit digest */
void jh_256(const void *input, size_t inputbitlen, uint8_t *digest);
