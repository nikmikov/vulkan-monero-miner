#pragma once

#include <stddef.h>
#include <stdint.h>

#define CRYPTONIGHT_HASH_LENGTH 256

struct cryptonight_hash {
  uint8_t data[CRYPTONIGHT_HASH_LENGTH];
};

struct cryptonight_ctx;

struct cryptonight_ctx *cryptonight_ctx_new();

void cryptonight_ctx_free(struct cryptonight_ctx **);

void cryptonight_aesni(const uint8_t *input, size_t input_size,
                       struct cryptonight_hash *output,
                       struct cryptonight_ctx *ctx0);
