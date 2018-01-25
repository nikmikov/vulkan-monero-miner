/*
 * Skein-256 hash function
 *
 * Source code author: Doug Whiting, 2008.
 */
#pragma once

#include <stdint.h>

#define SKEIN_256_HASH_BIT_LEN 256
#define SKEIN_256_BLOCK_SIZE_BYTES 32
#define SKEIN_256_STATE_WORDS (SKEIN_256_BLOCK_SIZE_BYTES / 8)
#define SKEIN_256_ROUNDS_TOTAL 72
#define SKEIN_KS_PARITY 0x1BD11BDAA9FC1A22

struct skein256_state {
  uint64_t len;
  /* chaining variables */
  uint64_t chaining[SKEIN_256_STATE_WORDS]; // X

  /* tweak words: T[0]=byte cnt, T[1]=flags */
  uint64_t T[2];

  /* partial block buffer (8-byte aligned) */
  uint8_t buffer[SKEIN_256_BLOCK_SIZE_BYTES]; // b
};

void skein256_init(struct skein256_state *state);
void skein256_update(struct skein256_state *state, const void *data,
                     uint64_t databitlen);
void skein256_final(struct skein256_state *state, uint8_t *digest);

void skein256_hash(const void *in, uint64_t inlen_bytes, uint8_t *digest);
