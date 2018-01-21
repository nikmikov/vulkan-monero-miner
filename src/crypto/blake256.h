#pragma once

#include <stdint.h>

void blake256_hash(uint8_t *out, const uint8_t *in, uint64_t inlen);
