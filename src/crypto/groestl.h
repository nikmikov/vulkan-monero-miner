/*
 * Based on AES-NI Groestl-256
 * Author: Krystian Matusiewicz, Günther A. Roland, Martin Schläffer
 *
 * This code is placed in the public domain
 */
#pragma once

#include <stdalign.h>
#include <stdint.h>
#include <stddef.h>

#define GROESTL256_HASH_BIT_LEN 256
#define GROESTL256_COLS 8
#define GROESTL256_ROWS 8
#define GROESTL256_BLOCK_SIZE (GROESTL256_ROWS * GROESTL256_COLS)
#define GROESTL256_ROUNDS 10

struct groestl_state {
  alignas(32) uint64_t chaining[GROESTL256_BLOCK_SIZE / 8];
  alignas(32) uint8_t buffer[GROESTL256_BLOCK_SIZE];
  uint64_t block_counter;
  int buf_ptr;
  int bits_in_last_byte;
};

/** stream processing */
void groestl_256_init(struct groestl_state *state);
void groestl_256_update(struct groestl_state *state, const void *data,
                       size_t databitlen);
void groestl_256_final(struct groestl_state *state, uint8_t *digest);

/** hash fixed size input and produce 256-bit digest */
void groestl_256(const void *input, size_t inputbitlen, uint8_t *digest);
