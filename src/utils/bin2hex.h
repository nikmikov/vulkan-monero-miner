#pragma once

#include <stddef.h>

static inline char hf_bin2hex(uint8_t c)
{
  if (c <= 0x9) {
    return '0' + c;
  } else {
    return 'a' - 0xA + c;
  }
}

/** Convert binary string to hex representation */
static inline void bin2hex(const void *in, size_t len, char *out)
{
  const uint8_t *p = in;
  for (size_t i = 0; i < len; ++i) {
    out[i * 2] = hf_bin2hex((p[i] & 0xF0) >> 4);
    out[i * 2 + 1] = hf_bin2hex(p[i] & 0x0F);
  }
}
