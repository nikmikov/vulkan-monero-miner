#ifndef CRYPTONIGHT_H
#define CRYPTONIGHT_H

#include <stddef.h>
#include <stdint.h>

#define CRYPTONIGHT_HASH_LENGTH 256

struct cryptonight_ctx {
    uint8_t hash_state[224]; // Need only 200, explicit align
    uint8_t* long_state;
    uint8_t ctx_info[24];     //Use some of the extra memory for flags
};

struct cryptonight_hash {
    uint8_t data[CRYPTONIGHT_HASH_LENGTH];
};

void cryptonight_aesni(const uint8_t *input,
                       size_t input_size,
                       struct cryptonight_hash *output,
                       struct cryptonight_ctx *ctx0);

#endif /** CRYPTONIGHT_H */
