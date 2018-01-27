/*
 * Skein-256 hash function
 *
 * Source code author: Doug Whiting, 2008.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

#define SKEIN_256_HASH_BIT_LEN 256
#define SKEIN_256_BLOCK_SIZE 32
#define SKEIN_256_STATE_WORDS (SKEIN_256_BLOCK_SIZE / 8)
#define SKEIN_256_ROUNDS_TOTAL 72
#define SKEIN_KS_PARITY 0x1BD11BDAA9FC1A22

struct skein_256_state {
  /* chaining variables */
  uint64_t chaining[SKEIN_256_STATE_WORDS];

  /* tweak words: T[0]=byte cnt, T[1]=flags */
  uint64_t T[2];

  /* partial block buffer (8-byte aligned) */
  uint8_t buffer[SKEIN_256_BLOCK_SIZE];

  size_t block_counter;
  size_t buf_ptr;
  int bits_in_last_byte;
};

/** stream processing */
void skein_256_init(struct skein_256_state *state);
void skein_256_update(struct skein_256_state *state, const void *data,
                      size_t databitlen);
void skein_256_final(struct skein_256_state *state, uint8_t *digest);

/** hash fixed size input and produce 256-bit digest */
void skein_256(const void *input, size_t inputbitlen, uint8_t *digest);
