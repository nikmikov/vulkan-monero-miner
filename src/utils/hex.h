#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/** convert binary value to hex character */
static inline char hf_bin2hex(uint8_t c)
{
  assert(c <= 0xf);
  if (c <= 0x9) {
    return '0' + c;
  } else {
    return 'a' - 0xA + c;
  }
}

/** convert hex character to binary.
 *  return 0xff if input is not valid hex character */
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

/** convert binary array of size `len` to hex string */
static inline void hex_from_binary(const void *in, size_t len, char *out)
{
  const uint8_t *p = in;
  for (size_t i = 0; i < len; ++i) {
    out[i * 2] = hf_bin2hex((p[i] & 0xF0) >> 4);
    out[i * 2 + 1] = hf_bin2hex(p[i] & 0x0F);
  }
}

/** convert hex string of size `len` to binary array
    return number of bytes in the result, will return '0' in case of error  */
static inline size_t hex_to_binary(const char *in, size_t len, uint8_t *out)
{
  size_t lpad = len & 1;
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
