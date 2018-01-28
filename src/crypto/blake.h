#pragma once

#include <stdint.h>

struct blake_state {
  uint32_t h[8], s[4], t[2];
  int buflen, nullt;
  uint8_t buf[64];
};

/** stream processing */
void blake_256_init(struct blake_state *state);
void blake_256_update(struct blake_state *state, const void *data,
                      size_t databitlen);
void blake_256_final(struct blake_state *state, uint8_t *digest);

/** hash fixed size input and produce 256-bit digest */
void blake_256(const void *input, size_t inputbitlen, uint8_t *digest);
