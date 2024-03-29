#include "crypto/cryptonight/cryptonight.h"

#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdlib.h>

#include "crypto/aes.h"
#include "crypto/blake.h"
#include "crypto/groestl.h"
#include "crypto/jh.h"
#include "crypto/keccak-tiny.h"
#include "crypto/skein.h"
#include "logging.h"
#include "utils/hugepages.h"

#define CRYPTONIGHT_MEMORY 2097152                         /* 2 MiB */
#define CRYPTONIGHT_MEMORY_M128I (CRYPTONIGHT_MEMORY >> 4) /* 2 MiB / 16 */
#define CRYPTONIGHT_ITERATIONS 0x80000                     /** 524288 */
#define CRYPTONIGHT_MASK 0x1FFFF0                          /** for monero */

struct cryptonight_ctx {
  uint8_t *long_state;
  alignas(16) uint8_t hash_state[200];
  bool is_hugepages_mem;
  bool is_mlocked_mem;
};

#define AES_GENKEY_SUB(rcon, xout0, xout2)                                     \
  {                                                                            \
    __m128i xout1 = aes_keygenassist(xout2, (uint8_t)rcon);                    \
    xout1 = _mm_shuffle_epi32(xout1, 0xFF);                                    \
    xout0 = sl_xor(xout0);                                                     \
    xout0 = _mm_xor_si128(xout0, xout1);                                       \
    xout1 = aes_keygenassist(xout0, 0x00);                                     \
    xout1 = _mm_shuffle_epi32(xout1, 0xAA);                                    \
    xout2 = sl_xor(xout2);                                                     \
    xout2 = _mm_xor_si128(xout2, xout1);                                       \
  }

static inline uint64_t _umul128(uint64_t a, uint64_t b, uint64_t *hi)
{
  unsigned __int128 r = (unsigned __int128)a * (unsigned __int128)b;
  *hi = r >> 64;
  return (uint64_t)r;
}

// This will shift and xor tmp1 into itself as 4 32-bit vals such as
// sl_xor(a1 a2 a3 a4) = a1 (a2^a1) (a3^a2^a1) (a4^a3^a2^a1)
static inline __m128i sl_xor(__m128i tmp1)
{
  __m128i tmp4;
  tmp4 = _mm_slli_si128(tmp1, 0x04);
  tmp1 = _mm_xor_si128(tmp1, tmp4);
  tmp4 = _mm_slli_si128(tmp4, 0x04);
  tmp1 = _mm_xor_si128(tmp1, tmp4);
  tmp4 = _mm_slli_si128(tmp4, 0x04);
  tmp1 = _mm_xor_si128(tmp1, tmp4);
  return tmp1;
}

static inline void aes_round(__m128i key, __m128i *x0, __m128i *x1, __m128i *x2,
                             __m128i *x3, __m128i *x4, __m128i *x5, __m128i *x6,
                             __m128i *x7)
{
  *x0 = aes_encode(*x0, key);
  *x1 = aes_encode(*x1, key);
  *x2 = aes_encode(*x2, key);
  *x3 = aes_encode(*x3, key);
  *x4 = aes_encode(*x4, key);
  *x5 = aes_encode(*x5, key);
  *x6 = aes_encode(*x6, key);
  *x7 = aes_encode(*x7, key);
}

static inline void aes_genkey(const __m128i *memory, __m128i *k0, __m128i *k1,
                              __m128i *k2, __m128i *k3, __m128i *k4,
                              __m128i *k5, __m128i *k6, __m128i *k7,
                              __m128i *k8, __m128i *k9)
{
  __m128i xout0, xout2;

  xout0 = _mm_load_si128(memory);
  xout2 = _mm_load_si128(memory + 1);
  *k0 = xout0;
  *k1 = xout2;

  AES_GENKEY_SUB(0x01, xout0, xout2);
  *k2 = xout0;
  *k3 = xout2;

  AES_GENKEY_SUB(0x02, xout0, xout2);
  *k4 = xout0;
  *k5 = xout2;

  AES_GENKEY_SUB(0x04, xout0, xout2);
  *k6 = xout0;
  *k7 = xout2;

  AES_GENKEY_SUB(0x08, xout0, xout2);
  *k8 = xout0;
  *k9 = xout2;
}

void cn_explode_scratchpad(const __m128i *input, __m128i *output)
{
  // This is more than we have registers, compiler will assign 2 keys on the
  // stack
  __m128i xin0, xin1, xin2, xin3, xin4, xin5, xin6, xin7;
  __m128i k0, k1, k2, k3, k4, k5, k6, k7, k8, k9;

  aes_genkey(input, &k0, &k1, &k2, &k3, &k4, &k5, &k6, &k7, &k8, &k9);

  xin0 = _mm_load_si128(input + 4);
  xin1 = _mm_load_si128(input + 5);
  xin2 = _mm_load_si128(input + 6);
  xin3 = _mm_load_si128(input + 7);
  xin4 = _mm_load_si128(input + 8);
  xin5 = _mm_load_si128(input + 9);
  xin6 = _mm_load_si128(input + 10);
  xin7 = _mm_load_si128(input + 11);

  for (size_t i = 0; i < CRYPTONIGHT_MEMORY_M128I; i += 8) {

    aes_round(k0, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
    aes_round(k1, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
    aes_round(k2, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
    aes_round(k3, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
    aes_round(k4, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
    aes_round(k5, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
    aes_round(k6, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
    aes_round(k7, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
    aes_round(k8, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);
    aes_round(k9, &xin0, &xin1, &xin2, &xin3, &xin4, &xin5, &xin6, &xin7);

    _mm_store_si128(output + i + 0, xin0);
    _mm_store_si128(output + i + 1, xin1);
    _mm_store_si128(output + i + 2, xin2);
    _mm_store_si128(output + i + 3, xin3);

    //    if(PREFETCH)
    //  _mm_prefetch((const char*)output + i + 0, _MM_HINT_T2);

    _mm_store_si128(output + i + 4, xin4);
    _mm_store_si128(output + i + 5, xin5);
    _mm_store_si128(output + i + 6, xin6);
    _mm_store_si128(output + i + 7, xin7);

    // if(PREFETCH)
    //  _mm_prefetch((const char*)output + i + 4, _MM_HINT_T2);
  }
}

void cn_implode_scratchpad(const __m128i *input, __m128i *output)
{
  // This is more than we have registers, compiler will assign 2 keys on the
  // stack
  __m128i xout0, xout1, xout2, xout3, xout4, xout5, xout6, xout7;
  __m128i k0, k1, k2, k3, k4, k5, k6, k7, k8, k9;

  aes_genkey(output + 2, &k0, &k1, &k2, &k3, &k4, &k5, &k6, &k7, &k8, &k9);

  xout0 = _mm_load_si128(output + 4);
  xout1 = _mm_load_si128(output + 5);
  xout2 = _mm_load_si128(output + 6);
  xout3 = _mm_load_si128(output + 7);
  xout4 = _mm_load_si128(output + 8);
  xout5 = _mm_load_si128(output + 9);
  xout6 = _mm_load_si128(output + 10);
  xout7 = _mm_load_si128(output + 11);

  for (size_t i = 0; i < CRYPTONIGHT_MEMORY_M128I; i += 8) {

    xout0 = _mm_xor_si128(_mm_load_si128(input + i + 0), xout0);
    xout1 = _mm_xor_si128(_mm_load_si128(input + i + 1), xout1);
    xout2 = _mm_xor_si128(_mm_load_si128(input + i + 2), xout2);
    xout3 = _mm_xor_si128(_mm_load_si128(input + i + 3), xout3);

    xout4 = _mm_xor_si128(_mm_load_si128(input + i + 4), xout4);
    xout5 = _mm_xor_si128(_mm_load_si128(input + i + 5), xout5);
    xout6 = _mm_xor_si128(_mm_load_si128(input + i + 6), xout6);
    xout7 = _mm_xor_si128(_mm_load_si128(input + i + 7), xout7);

    aes_round(k0, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
    aes_round(k1, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
    aes_round(k2, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
    aes_round(k3, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
    aes_round(k4, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
    aes_round(k5, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
    aes_round(k6, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
    aes_round(k7, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
    aes_round(k8, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
    aes_round(k9, &xout0, &xout1, &xout2, &xout3, &xout4, &xout5, &xout6,
              &xout7);
  }

  _mm_store_si128(output + 4, xout0);
  _mm_store_si128(output + 5, xout1);
  _mm_store_si128(output + 6, xout2);
  _mm_store_si128(output + 7, xout3);
  _mm_store_si128(output + 8, xout4);
  _mm_store_si128(output + 9, xout5);
  _mm_store_si128(output + 10, xout6);
  _mm_store_si128(output + 11, xout7);
}

static inline uint64_t get_monero_tweak_const(const uint8_t *input,
                                              const uint8_t *state)
{
  uint64_t inp = *((const uint64_t *)&input[35]);
  uint64_t st = ((const uint64_t *)state)[24];
  return  st ^ inp;
}

static inline void cryptonight_monero_tweak(uint64_t *mem_out, __m128i tmp)
{
  mem_out[0] = _mm_cvtsi128_si64(tmp);

  tmp = _mm_castps_si128(
      _mm_movehl_ps(_mm_castsi128_ps(tmp), _mm_castsi128_ps(tmp)));
  uint64_t vh = _mm_cvtsi128_si64(tmp);

  uint8_t x = (uint8_t)(vh >> 24);
  static const uint16_t table = 0x7531;
  const uint8_t index = (((x >> 3) & 6) | (x & 1)) << 1;
  vh ^= ((table >> index) & 0x3) << 28;

  mem_out[1] = vh;
}

void cryptonight_aesni(const uint8_t *input, size_t input_size,
                       struct cryptonight_hash *output,
                       struct cryptonight_ctx *ctx0)
{
  assert(input != NULL);
  assert(output != NULL);
  assert(ctx0 != NULL);

  // init scratchpad
  keccak_256(ctx0->hash_state, 200, input, input_size);

  // monero pow v7 const
  const uint64_t monero_tweak_const =
      get_monero_tweak_const(input, ctx0->hash_state);

  cn_explode_scratchpad((__m128i *)ctx0->hash_state,
                        (__m128i *)ctx0->long_state);

  uint8_t *l0 = ctx0->long_state;
  uint64_t *h0 = (uint64_t *)ctx0->hash_state;

  uint64_t al0 = h0[0] ^ h0[4];
  uint64_t ah0 = h0[1] ^ h0[5];
  __m128i bx0 = _mm_set_epi64x(h0[3] ^ h0[7], h0[2] ^ h0[6]);

  uint64_t idx0 = h0[0] ^ h0[4];

  for (size_t i = 0; i < CRYPTONIGHT_ITERATIONS; ++i) {
    __m128i cx;
    cx = _mm_load_si128((__m128i *)&l0[idx0 & CRYPTONIGHT_MASK]);
    cx = aes_encode(cx, _mm_set_epi64x(ah0, al0));

    // new in monero pow v7
    cryptonight_monero_tweak((uint64_t *)&l0[idx0 & CRYPTONIGHT_MASK],
                             _mm_xor_si128(bx0, cx));

    idx0 = _mm_cvtsi128_si64(cx);
    bx0 = cx;

    uint64_t hi, lo, cl, ch;
    cl = ((uint64_t *)&l0[idx0 & CRYPTONIGHT_MASK])[0];
    ch = ((uint64_t *)&l0[idx0 & CRYPTONIGHT_MASK])[1];

    lo = _umul128(idx0, cl, &hi);

    al0 += hi;
    ah0 += lo;
    ((uint64_t *)&l0[idx0 & CRYPTONIGHT_MASK])[0] = al0;
    ((uint64_t *)&l0[idx0 & CRYPTONIGHT_MASK])[1] = ah0 ^ monero_tweak_const;
    ah0 ^= ch;
    al0 ^= cl;
    idx0 = al0;
  }
  cn_implode_scratchpad((__m128i *)ctx0->long_state,
                        (__m128i *)ctx0->hash_state);

  keccak_f((uint64_t *)ctx0->hash_state, 24);

  static void (*const extra_hashes[4])(const void *, size_t, uint8_t *) = {
      blake_256, groestl_256, jh_256, skein_512_256};
  const int final_hash_idx = ctx0->hash_state[0] & 3;
  extra_hashes[final_hash_idx](ctx0->hash_state, 200 * 8, (uint8_t *)output);
}

struct cryptonight_ctx *cryptonight_ctx_new()
{
  struct cryptonight_ctx *ctx = calloc(1, sizeof(struct cryptonight_ctx));
  ctx->long_state = hugepages_alloc(CRYPTONIGHT_MEMORY);
  if (ctx->long_state == NULL) {
    // fallback to regular aligned alloc
    log_warn("Huge pages support unavaliable. Performance may suffer");
    int res =
        posix_memalign((void *)&ctx->long_state, CRYPTONIGHT_MEMORY, 4096);
    if (res != 0) {
      log_error("Memory allocation for context failed");
      free(ctx);
      return NULL;
    }
  } else {
    ctx->is_hugepages_mem = true;
  }

  if (madvise(ctx->long_state, CRYPTONIGHT_MEMORY,
              MADV_RANDOM | MADV_WILLNEED) != 0) {
    log_warn("madvise failed");
  }

  if (ctx->is_hugepages_mem &&
      mlock(ctx->long_state, CRYPTONIGHT_MEMORY) != 0) {
    log_warn("mlock failed");
    ctx->is_mlocked_mem = false;
  } else {
    ctx->is_mlocked_mem = true;
  }

  return ctx;
}

void cryptonight_ctx_free(struct cryptonight_ctx **ptr)
{
  if ((*ptr)->is_mlocked_mem) {
    munlock((*ptr)->long_state, CRYPTONIGHT_MEMORY);
  }
  if ((*ptr)->is_hugepages_mem) {
    hugepages_free((*ptr)->long_state, CRYPTONIGHT_MEMORY);
  } else {
    free((*ptr)->long_state);
  }
  free(*ptr);
  *ptr = NULL;
}
