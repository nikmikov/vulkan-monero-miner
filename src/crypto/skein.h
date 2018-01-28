/*
 * Skein-512-256 hash function
 *
 * Source code author: Doug Whiting, 2008.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#define SKEIN_512_BLOCK_SIZE 64
#define SKEIN_512_STATE_WORDS (SKEIN_512_BLOCK_SIZE / 8)

struct skein_512_state {
  /* chaining variables */
  uint64_t chaining[SKEIN_512_STATE_WORDS];

  /* tweak words: T[0]=byte cnt, T[1]=flags */
  uint64_t T[2];

  /* partial block buffer (8-byte aligned) */
  uint8_t buffer[SKEIN_512_BLOCK_SIZE];

  size_t buf_ptr;
  int bits_in_last_byte;
};

/** stream processing */
void skein_512_256_init(struct skein_512_state *state);
void skein_512_update(struct skein_512_state *state, const void *data,
                      size_t databitlen);
void skein_512_final(struct skein_512_state *state, size_t digestlenbits,
                     uint8_t *digest);

/** hash fixed size input with skein-512 and produce 256-bit digest */
void skein_512_256(const void *input, size_t inputbitlen, uint8_t *digest);
