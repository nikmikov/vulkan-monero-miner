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

// this is hardcoded at the moment, but really shouldn't be
#define MONERO_NONCE_POS 39

#define HASH_STATE_SIZE 200
#define INPUT_HASH_SIZE 88

#define CRYPTONIGHT_MEMORY_UINT (CRYPTONIGHT_MEMORY >> 2) /* 2 MiB / 4 */

// Keccak constants
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


// AES constants
//
#define aes_data(w) {\
    w(0x63), w(0x7c), w(0x77), w(0x7b), w(0xf2), w(0x6b), w(0x6f), w(0xc5),\
    w(0x30), w(0x01), w(0x67), w(0x2b), w(0xfe), w(0xd7), w(0xab), w(0x76),\
    w(0xca), w(0x82), w(0xc9), w(0x7d), w(0xfa), w(0x59), w(0x47), w(0xf0),\
    w(0xad), w(0xd4), w(0xa2), w(0xaf), w(0x9c), w(0xa4), w(0x72), w(0xc0),\
    w(0xb7), w(0xfd), w(0x93), w(0x26), w(0x36), w(0x3f), w(0xf7), w(0xcc),\
    w(0x34), w(0xa5), w(0xe5), w(0xf1), w(0x71), w(0xd8), w(0x31), w(0x15),\
    w(0x04), w(0xc7), w(0x23), w(0xc3), w(0x18), w(0x96), w(0x05), w(0x9a),\
    w(0x07), w(0x12), w(0x80), w(0xe2), w(0xeb), w(0x27), w(0xb2), w(0x75),\
    w(0x09), w(0x83), w(0x2c), w(0x1a), w(0x1b), w(0x6e), w(0x5a), w(0xa0),\
    w(0x52), w(0x3b), w(0xd6), w(0xb3), w(0x29), w(0xe3), w(0x2f), w(0x84),\
    w(0x53), w(0xd1), w(0x00), w(0xed), w(0x20), w(0xfc), w(0xb1), w(0x5b),\
    w(0x6a), w(0xcb), w(0xbe), w(0x39), w(0x4a), w(0x4c), w(0x58), w(0xcf),\
    w(0xd0), w(0xef), w(0xaa), w(0xfb), w(0x43), w(0x4d), w(0x33), w(0x85),\
    w(0x45), w(0xf9), w(0x02), w(0x7f), w(0x50), w(0x3c), w(0x9f), w(0xa8),\
    w(0x51), w(0xa3), w(0x40), w(0x8f), w(0x92), w(0x9d), w(0x38), w(0xf5),\
    w(0xbc), w(0xb6), w(0xda), w(0x21), w(0x10), w(0xff), w(0xf3), w(0xd2),\
    w(0xcd), w(0x0c), w(0x13), w(0xec), w(0x5f), w(0x97), w(0x44), w(0x17),\
    w(0xc4), w(0xa7), w(0x7e), w(0x3d), w(0x64), w(0x5d), w(0x19), w(0x73),\
    w(0x60), w(0x81), w(0x4f), w(0xdc), w(0x22), w(0x2a), w(0x90), w(0x88),\
    w(0x46), w(0xee), w(0xb8), w(0x14), w(0xde), w(0x5e), w(0x0b), w(0xdb),\
    w(0xe0), w(0x32), w(0x3a), w(0x0a), w(0x49), w(0x06), w(0x24), w(0x5c),\
    w(0xc2), w(0xd3), w(0xac), w(0x62), w(0x91), w(0x95), w(0xe4), w(0x79),\
    w(0xe7), w(0xc8), w(0x37), w(0x6d), w(0x8d), w(0xd5), w(0x4e), w(0xa9),\
    w(0x6c), w(0x56), w(0xf4), w(0xea), w(0x65), w(0x7a), w(0xae), w(0x08),\
    w(0xba), w(0x78), w(0x25), w(0x2e), w(0x1c), w(0xa6), w(0xb4), w(0xc6),\
    w(0xe8), w(0xdd), w(0x74), w(0x1f), w(0x4b), w(0xbd), w(0x8b), w(0x8a),\
    w(0x70), w(0x3e), w(0xb5), w(0x66), w(0x48), w(0x03), w(0xf6), w(0x0e),\
    w(0x61), w(0x35), w(0x57), w(0xb9), w(0x86), w(0xc1), w(0x1d), w(0x9e),\
    w(0xe1), w(0xf8), w(0x98), w(0x11), w(0x69), w(0xd9), w(0x8e), w(0x94),\
    w(0x9b), w(0x1e), w(0x87), w(0xe9), w(0xce), w(0x55), w(0x28), w(0xdf),\
    w(0x8c), w(0xa1), w(0x89), w(0x0d), w(0xbf), w(0xe6), w(0x42), w(0x68),\
    w(0x41), w(0x99), w(0x2d), w(0x0f), w(0xb0), w(0x54), w(0xbb), w(0x16) }

#define AES_WPOLY           0x011b

#define aes_b2w(b0, b1, b2, b3) (((uint)(b3) << 24) | \
    ((uint)(b2) << 16) | ((uint)(b1) << 8) | (b0))

#define aes_f2(x)   ((x<<1) ^ (((x>>7) & 1) * AES_WPOLY))
#define aes_f3(x)   (aes_f2(x) ^ x)
#define aes_h0(x)   (x)

#define aes_u0(p)   aes_b2w(aes_f2(p),         p,         p, aes_f3(p))
#define aes_u1(p)   aes_b2w(aes_f3(p), aes_f2(p),         p,         p)
#define aes_u2(p)   aes_b2w(        p, aes_f3(p), aes_f2(p),         p)
#define aes_u3(p)   aes_b2w(        p,         p, aes_f3(p), aes_f2(p))

static const constant uint aes_table[4][256] = {
  aes_data(aes_u0), aes_data(aes_u1), aes_data(aes_u2), aes_data(aes_u3)
};

static const constant uint aes_sbox[256] = aes_data(aes_h0);

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

#define BYTE0(a) (a & 0xff)
#define BYTE1(a) ((a >> 8) & 0xff)
#define BYTE2(a) ((a >> 16) & 0xff)
#define BYTE3(a) ((a >> 24) & 0xff)

static inline uint sub_word(uint4 key)
{
  return (aes_sbox[key.s3] << 24) |
    (aes_sbox[key.s2] << 16 )     |
    (aes_sbox[key.s1] << 8  )     |
    aes_sbox[key.s0];
}


#define ROTR8(x) (x >> 8) | (x << (32 - 8))

uint4 aes_keygenassist(uint4 key, uint rcon)
{
  uint X1 = sub_word(BYTE1(key));
  uint X3 = sub_word(BYTE3(key));
  uint4 res = {ROTR8(X3) ^ rcon,  X3, ROTR8(X1) ^ rcon, X1 };
  return res;
}

// This will shift and xor tmp1 into itself as 4 32-bit vals such as
// sl_xor(a1 a2 a3 a4) = a1 (a2^a1) (a3^a2^a1) (a4^a3^a2^a1)
static inline uint4 sl_xor(uint4 a)
{
  return a ^ (a << 4) ^ (a << 8) ^ (a << 12);
}

uint8 aes_genkey8(const uint8 k0, const uint rcon)
{
  union {
    struct {
      uint4 xout0, xout1;
    };
    uint8 res;
  } X = {.res = k0};

  X.xout0 = sl_xor(X.xout0) ^ BYTE3(aes_keygenassist(X.xout1, rcon));
  X.xout1 = sl_xor(X.xout1) ^ BYTE2(aes_keygenassist(X.xout0, 0x00));

  return X.res;
}


uint8 aes_round8(const uint8 a, const uint8 key)
{
  uint8 i0 = BYTE0(a);
  uint8 i1 = BYTE1(a);
  uint8 i2 = BYTE2(a);
  uint8 i3 = BYTE3(a);

  uint8 x = {
     aes_table[0][i0.s3] ^ aes_table[1][i1.s0] ^ aes_table[2][i2.s1] ^ aes_table[3][i2.s2],
     aes_table[0][i0.s2] ^ aes_table[1][i2.s3] ^ aes_table[2][i2.s0] ^ aes_table[3][i2.s1],
     aes_table[0][i0.s1] ^ aes_table[1][i1.s2] ^ aes_table[2][i2.s3] ^ aes_table[3][i2.s0],
     aes_table[0][i0.s0] ^ aes_table[1][i1.s1] ^ aes_table[2][i2.s2] ^ aes_table[3][i2.s3],
     aes_table[0][i0.s7] ^ aes_table[1][i1.s4] ^ aes_table[2][i2.s5] ^ aes_table[3][i2.s6],
     aes_table[0][i0.s6] ^ aes_table[1][i2.s7] ^ aes_table[2][i2.s4] ^ aes_table[3][i2.s5],
     aes_table[0][i0.s5] ^ aes_table[1][i1.s6] ^ aes_table[2][i2.s7] ^ aes_table[3][i2.s4],
     aes_table[0][i0.s4] ^ aes_table[1][i1.s5] ^ aes_table[2][i2.s6] ^ aes_table[3][i2.s7]
  };

  return x ^ key;
}

#define AES_ROUND8_4(k,x0,x1,x2,x3) \
  x0 = aes_round8(k, x0); \
  x1 = aes_round8(k, x1); \
  x2 = aes_round8(k, x2); \
  x3 = aes_round8(k, x3);

uint4 aes_round4(const uint4 a, const uint4 key)
{
  uint4 i0 = BYTE0(a);
  uint4 i1 = BYTE1(a);
  uint4 i2 = BYTE2(a);
  uint4 i3 = BYTE3(a);

  uint4 x = {
     aes_table[0][i0.s3] ^ aes_table[1][i1.s0] ^ aes_table[2][i2.s1] ^ aes_table[3][i2.s2],
     aes_table[0][i0.s2] ^ aes_table[1][i2.s3] ^ aes_table[2][i2.s0] ^ aes_table[3][i2.s1],
     aes_table[0][i0.s1] ^ aes_table[1][i1.s2] ^ aes_table[2][i2.s3] ^ aes_table[3][i2.s0],
     aes_table[0][i0.s0] ^ aes_table[1][i1.s1] ^ aes_table[2][i2.s2] ^ aes_table[3][i2.s3]
  };

  return x ^ key;
}

void explode_scratchpad(uint *state, uint *scratchpad)
{
  uint8 xin0, xin1, xin2, xin3;
  uint8 k0, k1, k2, k3, k4;

  // bytes 0..31 of the Keccak final state are
  // interpreted as an AES-256 key and expanded to 10 round keys.
  k0 = vload8(0, state);
  k1 = aes_genkey8(k0, 0x01);
  k2 = aes_genkey8(k1, 0x02);
  k3 = aes_genkey8(k2, 0x04);
  k4 = aes_genkey8(k3, 0x08);

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
    AES_ROUND8_4(k0, xin0, xin1, xin2, xin3);
    AES_ROUND8_4(k1, xin0, xin1, xin2, xin3);
    AES_ROUND8_4(k2, xin0, xin1, xin2, xin3);
    AES_ROUND8_4(k3, xin0, xin1, xin2, xin3);
    AES_ROUND8_4(k4, xin0, xin1, xin2, xin3);

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
  k1 = aes_genkey8(k0, 0x01);
  k2 = aes_genkey8(k1, 0x02);
  k3 = aes_genkey8(k2, 0x04);
  k4 = aes_genkey8(k3, 0x08);

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

    AES_ROUND8_4(k0, xin0, xin1, xin2, xin3);
    AES_ROUND8_4(k1, xin0, xin1, xin2, xin3);
    AES_ROUND8_4(k2, xin0, xin1, xin2, xin3);
    AES_ROUND8_4(k3, xin0, xin1, xin2, xin3);
    AES_ROUND8_4(k4, xin0, xin1, xin2, xin3);
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
  ulong4 x0 = vload4(0, hash_state) ^ vload4(4, hash_state);

  // run main cryptonight loop
  memory_hard_loop(x0, scratchpad);

  // implode scratchpad
  implode_scratchpad((uint *)hash_state, scratchpad);

  // keccak of 24 bytes of final hash
  keccakf1600(hash_state);

  // copy to output
  const size_t offs = get_global_id(0) * HASH_STATE_SIZE_ULONG;
  for (size_t i = offs; i < offs + HASH_STATE_SIZE_ULONG; ++i) {
    output[i] = hash_state[i];
  }
}
