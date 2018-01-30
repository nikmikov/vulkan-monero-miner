#pragma once

#include <stddef.h>
#include <stdint.h>

static inline uint8_t hf_hex2bin(char c)
{
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 0xA;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 0xA;
  }

  return 0xff;
}

static inline size_t hex2bin(const char *in, size_t len, uint8_t *out)
{
  size_t lpad = len % 2;
  size_t res = lpad;
  for (size_t i = lpad; i < len; i += 2, ++res) {
    uint8_t n0 = hf_hex2bin(in[i]);
    uint8_t n1 = hf_hex2bin(in[i + 1]);
    if (n0 > 0xf || n1 > 0xf) {
      return 0;
    }
    out[res] = (n0 << 4) | n1;
  }
  if (lpad) {
    out[0] = hf_hex2bin(in[0]);
    if (out[0] > 0xf) {
      return 0;
    }
  }
  return res;
}
