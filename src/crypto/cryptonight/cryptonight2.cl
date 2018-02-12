/** tunable parameters, can be overriden via -D flag to compiler */
#ifndef CRYPTONIGHT_MEMORY
#define CRYPTONIGHT_MEMORY 2097152 /* 2 MiB */
#endif

#ifndef CRYPTONIGHT_ITERATIONS
#define CRYPTONIGHT_ITERATIONS 0x80000 /** 524288 */
#endif

#ifndef CRYPTONIGHT_MASK
#define CRYPTONIGHT_MASK 0x1FFFF0
#endif

#define MONERO_NONCE_POS 39

#define HASH_STATE_SIZE 200
#define INPUT_HASH_SIZE 88

#define CRYPTONIGHT_MEMORY_UINT (CRYPTONIGHT_MEMORY >> 2) /* 2 MiB / 4 */

static const constant uint keccakf_rotc[24] = {1,  3,  6,  10, 15, 21, 28, 36,
                                               45, 55, 2,  14, 27, 41, 56, 8,
                                               25, 43, 62, 18, 39, 61, 20, 44};

static const constant uint keccakf_piln[24] = {10, 7,  11, 17, 18, 3,  5,  16,
                                               8,  21, 24, 4,  15, 23, 19, 13,
                                               12, 2,  20, 14, 22, 9,  6,  1};

static const constant ulong keccakf_rndc[24] = {
    0x0000000000000001, 0x0000000000008082, 0x800000000000808a,
    0x8000000080008000, 0x000000000000808b, 0x0000000080000001,
    0x8000000080008081, 0x8000000000008009, 0x000000000000008a,
    0x0000000000000088, 0x0000000080008009, 0x000000008000000a,
    0x000000008000808b, 0x800000000000008b, 0x8000000000008089,
    0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
    0x000000000000800a, 0x800000008000000a, 0x8000000080008081,
    0x8000000000008080, 0x0000000080000001, 0x8000000080008008};

void keccakf1600(ulong *st)
{
  int i, round;
  ulong t, bc[5];

#pragma unroll 1
  for (round = 0; round < 24; ++round) {
    // Theta
    bc[0] = st[0] ^ st[5] ^ st[10] ^ st[15] ^ st[20] ^
            rotate(st[2] ^ st[7] ^ st[12] ^ st[17] ^ st[22], 1UL);
    bc[1] = st[1] ^ st[6] ^ st[11] ^ st[16] ^ st[21] ^
            rotate(st[3] ^ st[8] ^ st[13] ^ st[18] ^ st[23], 1UL);
    bc[2] = st[2] ^ st[7] ^ st[12] ^ st[17] ^ st[22] ^
            rotate(st[4] ^ st[9] ^ st[14] ^ st[19] ^ st[24], 1UL);
    bc[3] = st[3] ^ st[8] ^ st[13] ^ st[18] ^ st[23] ^
            rotate(st[0] ^ st[5] ^ st[10] ^ st[15] ^ st[20], 1UL);
    bc[4] = st[4] ^ st[9] ^ st[14] ^ st[19] ^ st[24] ^
            rotate(st[1] ^ st[6] ^ st[11] ^ st[16] ^ st[21], 1UL);

    st[0] ^= bc[4];
    st[5] ^= bc[4];
    st[10] ^= bc[4];
    st[15] ^= bc[4];
    st[20] ^= bc[4];

    st[1] ^= bc[0];
    st[6] ^= bc[0];
    st[11] ^= bc[0];
    st[16] ^= bc[0];
    st[21] ^= bc[0];

    st[2] ^= bc[1];
    st[7] ^= bc[1];
    st[12] ^= bc[1];
    st[17] ^= bc[1];
    st[22] ^= bc[1];

    st[3] ^= bc[2];
    st[8] ^= bc[2];
    st[13] ^= bc[2];
    st[18] ^= bc[2];
    st[23] ^= bc[2];

    st[4] ^= bc[3];
    st[9] ^= bc[3];
    st[14] ^= bc[3];
    st[19] ^= bc[3];
    st[24] ^= bc[3];

    // Rho Pi
    t = st[1];
#pragma unroll
    for (i = 0; i < 24; ++i) {
      bc[0] = st[keccakf_piln[i]];
      st[keccakf_piln[i]] = rotate(t, (ulong)keccakf_rotc[i]);
      t = bc[0];
    }

#pragma unroll
    for (int i = 0; i < 25; i += 5) {
      ulong tmp1 = st[i], tmp2 = st[i + 1];

      st[i] = bitselect(st[i] ^ st[i + 2], st[i], st[i + 1]);
      st[i + 1] = bitselect(st[i + 1] ^ st[i + 3], st[i + 1], st[i + 2]);
      st[i + 2] = bitselect(st[i + 2] ^ st[i + 4], st[i + 2], st[i + 3]);
      st[i + 3] = bitselect(st[i + 3] ^ tmp1, st[i + 3], st[i + 4]);
      st[i + 4] = bitselect(st[i + 4] ^ tmp2, st[i + 4], tmp1);
    }

    //  Iota
    st[0] ^= keccakf_rndc[round];
  }
}

uint8 aes_genkey(const uint8 k0) { return k0; }

void aes_round8(const uint8 key, uint8 *x0, uint8 *x1, uint8 *x2, uint8 *x3) {}

uint4 aes_round4(const uint4 a, const uint4 key) {return a;}

void explode_scratchpad(uint *state, uint *scratchpad)
{
  uint8 xin0, xin1, xin2, xin3;
  uint8 k0, k1, k2, k3, k4;

  // bytes 0..31 of the Keccak final state are
  // interpreted as an AES-256 key and expanded to 10 round keys.
  k0 = vload8(0, state);
  k1 = aes_genkey(k0);
  k2 = aes_genkey(k1);
  k3 = aes_genkey(k2);
  k4 = aes_genkey(k3);

  // The bytes 64..191
  // are extracted from the Keccak final state and split into 8 blocks of
  // 16 bytes each.
  xin0 = vload8(2 * 8, state);
  xin1 = vload8(3 * 8, state);
  xin2 = vload8(4 * 8, state);
  xin3 = vload8(5 * 8, state);

  // Each block is encrypted using the following procedure
  // `block = aes_round(block, round_keys[i])`
  for (size_t i = 0; i < CRYPTONIGHT_MEMORY_UINT; i += 8 * 4) {
    aes_round8(k0, &xin0, &xin1, &xin2, &xin3);
    aes_round8(k1, &xin0, &xin1, &xin2, &xin3);
    aes_round8(k2, &xin0, &xin1, &xin2, &xin3);
    aes_round8(k3, &xin0, &xin1, &xin2, &xin3);
    aes_round8(k4, &xin0, &xin1, &xin2, &xin3);

    vstore8(xin0, i + 8 * 0, scratchpad);
    vstore8(xin1, i + 8 * 1, scratchpad);
    vstore8(xin2, i + 8 * 2, scratchpad);
    vstore8(xin3, i + 8 * 3, scratchpad);
  }
}

static inline size_t to_scratchpad_address(ulong v)
{
  return (v & CRYPTONIGHT_MASK) >> 2;
}

void memory_hard_loop(ulong4 k, uint *scratchpad)
{
  union {
    ulong4 v;
    struct {
      uint4 a, b;
    };
    struct {
      ulong a0, a1, b0, b1;
    };
  } X = {.v = k};

  for (size_t i = 0; i < CRYPTONIGHT_ITERATIONS; ++i) {
    // addr = to_scratchpad_address(a)
    const size_t idx_a = to_scratchpad_address(X.a0);

    // scratchpad[addr] = aes_round(scratchpad[addr], a)
    uint4 cx = aes_round4(vload4(idx_a, scratchpad), X.a);

    // b, scratchpad[addr] = scratchpad[addr], b ^ scratchpad[addr]
    vstore4(cx ^ X.b, idx_a, scratchpad);
    X.b = cx;

    // addr = to_scratchpad_address(b)
    const size_t idx_b = to_scratchpad_address(X.b0);

    // a = 8byte_add(a, 8byte_mul(b, scratchpad[addr]))
    cx = vload4(idx_b, scratchpad);
    ulong c0 = as_ulong2(cx).s0;
    X.a1 += X.b0 * c0;
    X.a0 += mul_hi(X.b0, c0);

    // a, scratchpad[addr] = a ^ scratchpad[addr], a
    vstore4(X.a, idx_b, scratchpad);
    X.a ^= cx;
  }
}

void implode_scratchpad(uint *state, uint *scratchpad)
{
  uint8 xin0, xin1, xin2, xin3;
  uint8 k0, k1, k2, k3, k4;
  // bytes 32..63 of the Keccak final state are
  // interpreted as an AES-256 key and expanded to 10 round keys.
  k0 = vload8(8, state);
  k1 = aes_genkey(k0);
  k2 = aes_genkey(k1);
  k3 = aes_genkey(k2);
  k4 = aes_genkey(k3);

  // Bytes 64..191 are extracted from the Keccak state
  xin0 = vload8(2 * 8, state) ^ vload8(0 * 8, scratchpad);
  xin1 = vload8(3 * 8, state) ^ vload8(1 * 8, scratchpad);
  xin2 = vload8(4 * 8, state) ^ vload8(2 * 8, scratchpad);
  xin3 = vload8(5 * 8, state) ^ vload8(3 * 8, scratchpad);

  // Then the result is encrypted in
  // the same manner as in the `explode_scratchpad`,
  // but using the new keys. The
  // result is XORed with the first 128 bytes from the scratchpad,
  // encrypted again, and so on.
  for (size_t i = 0; i < CRYPTONIGHT_MEMORY_UINT; i += 8 * 4) {
    xin0 ^= vload8(i + 8 * 0, scratchpad);
    xin1 ^= vload8(i + 8 * 1, scratchpad);
    xin2 ^= vload8(i + 8 * 2, scratchpad);
    xin3 ^= vload8(i + 8 * 3, scratchpad);

    aes_round8(k0, &xin0, &xin1, &xin2, &xin3);
    aes_round8(k1, &xin0, &xin1, &xin2, &xin3);
    aes_round8(k2, &xin0, &xin1, &xin2, &xin3);
    aes_round8(k3, &xin0, &xin1, &xin2, &xin3);
    aes_round8(k4, &xin0, &xin1, &xin2, &xin3);
  }

  // then the bytes 64..191 in the Keccak state
  // are replaced with the result
  vstore8(xin0, 2 * 8, state);
  vstore8(xin1, 3 * 8, state);
  vstore8(xin2, 4 * 8, state);
  vstore8(xin3, 5 * 8, state);
}

static const constant size_t INPUT_SIZE_ULONG =
    INPUT_HASH_SIZE / sizeof(ulong); // 11x8 == 88 bytes
static const constant size_t HASH_STATE_SIZE_ULONG =
    HASH_STATE_SIZE / sizeof(ulong);

void hash_state_init_with_nonce(global ulong *input, const uint nonce,
                                ulong *hash_state)
{
  for (size_t i = 0; i < INPUT_SIZE_ULONG; ++i) {
    hash_state[i] = input[i];
  }
  // insert nonce into the input hash
  // nonce is located in bytes 39..42 of input hash
  uint *nonce_ptr = (uint *)&((uchar *)hash_state)[MONERO_NONCE_POS];
  *nonce_ptr = nonce;
  for (size_t i = INPUT_SIZE_ULONG; i < HASH_STATE_SIZE_ULONG - 1; ++i) {
    hash_state[i] = 0UL;
  }
  // Last bit of padding
  hash_state[16] = 0x8000000000000000UL;
}

// input always 11x8 byte elements (88 bytes)
kernel void cryptonight(global ulong *input, const uint start_nonce,
                        global ulong *output)
{
  uint scratchpad[CRYPTONIGHT_MEMORY_UINT];
  ulong hash_state[HASH_STATE_SIZE_ULONG];

  uint nonce = start_nonce + get_global_id(0);
  // copy hash and insert work nonce
  hash_state_init_with_nonce(input, nonce, hash_state);

  // run keccak on input hash
  keccakf1600(hash_state);

  // init scratchpad
  explode_scratchpad((uint *)hash_state, scratchpad);

  // Bytes 0..31 and 32..63 of the Keccak state
  // are XORed, and the resulting 32 bytes are used to initialize
  // variables a and b, 16 bytes each.
  ulong4 x0  = vload4(0, hash_state) ^ vload4(4, hash_state);

  // run main cryptonight loop
  memory_hard_loop(x0, scratchpad);

  // implode scratchpad
  implode_scratchpad((uint *)hash_state, scratchpad);

  // keccak of 24 bytes of final hash
  keccakf1600(hash_state);

  // copy to output
  size_t output_offset = get_global_id(0) * HASH_STATE_SIZE_ULONG;
  for (size_t i = output_offset; i < output_offset + HASH_STATE_SIZE_ULONG; ++i) {
    output[i] = hash_state[i];
  }
}
