#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline char hf_hex2bin(char c, bool *err)
{
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 0xA;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 0xA;
  }

  *err = true;
  return 0;
}

static inline bool hex2bin(const char *in, size_t len, char *out)
{
  bool error = false;
  for (size_t i = 0; i < len; i += 2) {
    out[i / 2] =
        (hf_hex2bin(in[i], &error) << 4) | hf_hex2bin(in[i + 1], &error);
    if (error)
      return false;
  }
  return true;
}

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
