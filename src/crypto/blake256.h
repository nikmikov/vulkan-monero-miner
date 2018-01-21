#ifndef CRYPTO_BLAKE256_H
#define CRYPTO_BLAKE256_H

#include <stdint.h>

void blake256_hash(uint8_t *out, const uint8_t *in, uint64_t inlen);

#endif /* CRYPTO_BLAKE256_H */
