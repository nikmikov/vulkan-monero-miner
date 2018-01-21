/*
 * Based on AES-NI Groestl-256
 * Author: Krystian Matusiewicz, Günther A. Roland, Martin Schläffer
 *
 * This code is placed in the public domain
 */
#pragma once

#include <stdint.h>

#define GROESTL256_HASH_BIT_LEN 256
#define GROESTL256_COLS 8
#define GROESTL256_ROWS 8
#define GROESTL256_BLOCK_SIZE (GROESTL256_ROWS * GROESTL256_COLS)
#define GROESTL256_ROUNDS 10

struct groestl_state {
    uint64_t chaining[GROESTL256_BLOCK_SIZE / 8];
    uint8_t buffer[GROESTL256_BLOCK_SIZE];
    uint64_t block_counter;
    int buf_ptr;
    int bits_in_last_byte;
};

void groestl256_init(struct groestl_state *state);
void groestl256_update(struct groestl_state *state, const uint8_t *data, uint64_t databitlen);
void groestl256_final(struct groestl_state *state, uint8_t *digest);
void groestl256_hash(uint8_t *out, const uint8_t *in, uint64_t inlen_bytes);
