#pragma once
#define __STDC_WANT_LIB_EXT1__ 1
#include <stdint.h>
#include <stdlib.h>

// clang-format off

#define decshake(bits)                                                         \
  int shake##bits(uint8_t *, size_t, const uint8_t *, size_t);

#define decsha3(bits)                                                          \
  int sha3_##bits(uint8_t *, size_t, const uint8_t *, size_t);

#define deckeccak(bits)                                                        \
  int keccak_##bits(uint8_t *, size_t, const uint8_t *, size_t);

decshake(128)
decshake(256)

decsha3(224)
decsha3(256)
decsha3(384)
decsha3(512)

deckeccak(256)

// clang-format on

void keccak_f(uint64_t st[24], int rounds);
