/** Macros for AES SPV*/
#pragma once

// clang-format off

#define aes_sbox_const_enum                                                \
  CONST_UINT_AES_SBOX_0, CONST_UINT_AES_SBOX_4, CONST_UINT_AES_SBOX_8,     \
  CONST_UINT_AES_SBOX_12, CONST_UINT_AES_SBOX_16, CONST_UINT_AES_SBOX_20,  \
  CONST_UINT_AES_SBOX_24, CONST_UINT_AES_SBOX_28, CONST_UINT_AES_SBOX_32,  \
  CONST_UINT_AES_SBOX_36, CONST_UINT_AES_SBOX_40, CONST_UINT_AES_SBOX_44,  \
  CONST_UINT_AES_SBOX_48, CONST_UINT_AES_SBOX_52, CONST_UINT_AES_SBOX_56,  \
  CONST_UINT_AES_SBOX_60, CONST_UINT_AES_SBOX_64, CONST_UINT_AES_SBOX_68,  \
  CONST_UINT_AES_SBOX_72, CONST_UINT_AES_SBOX_76, CONST_UINT_AES_SBOX_80,  \
  CONST_UINT_AES_SBOX_84, CONST_UINT_AES_SBOX_88, CONST_UINT_AES_SBOX_92,  \
  CONST_UINT_AES_SBOX_96, CONST_UINT_AES_SBOX_100,                         \
  CONST_UINT_AES_SBOX_104, CONST_UINT_AES_SBOX_108,                        \
  CONST_UINT_AES_SBOX_112, CONST_UINT_AES_SBOX_116,                        \
  CONST_UINT_AES_SBOX_120, CONST_UINT_AES_SBOX_124,                        \
  CONST_UINT_AES_SBOX_128, CONST_UINT_AES_SBOX_132,                        \
  CONST_UINT_AES_SBOX_136, CONST_UINT_AES_SBOX_140,                        \
  CONST_UINT_AES_SBOX_144, CONST_UINT_AES_SBOX_148,                        \
  CONST_UINT_AES_SBOX_152, CONST_UINT_AES_SBOX_156,                        \
  CONST_UINT_AES_SBOX_160, CONST_UINT_AES_SBOX_164,                        \
  CONST_UINT_AES_SBOX_168, CONST_UINT_AES_SBOX_172,                        \
  CONST_UINT_AES_SBOX_176, CONST_UINT_AES_SBOX_180,                        \
  CONST_UINT_AES_SBOX_184, CONST_UINT_AES_SBOX_188,                        \
  CONST_UINT_AES_SBOX_192, CONST_UINT_AES_SBOX_196,                        \
  CONST_UINT_AES_SBOX_200, CONST_UINT_AES_SBOX_204,                        \
  CONST_UINT_AES_SBOX_208, CONST_UINT_AES_SBOX_212,                        \
  CONST_UINT_AES_SBOX_216, CONST_UINT_AES_SBOX_220,                        \
  CONST_UINT_AES_SBOX_224, CONST_UINT_AES_SBOX_228,                        \
  CONST_UINT_AES_SBOX_232, CONST_UINT_AES_SBOX_236,                        \
  CONST_UINT_AES_SBOX_240, CONST_UINT_AES_SBOX_244,                        \
  CONST_UINT_AES_SBOX_248, CONST_UINT_AES_SBOX_252

/** uint8_t aes_sbox[256] */
#define aes_sbox_const                                                     \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_0, 0x7b777c63,   \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_4, 0xc56f6bf2,   \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_8, 0x2b670130,   \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_12, 0x76abd7fe,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_16, 0x7dc982ca,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_20, 0xf04759fa,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_24, 0xafa2d4ad,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_28, 0xc072a49c,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_32, 0x2693fdb7,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_36, 0xccf73f36,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_40, 0xf1e5a534,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_44, 0x1531d871,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_48, 0xc323c704,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_52, 0x9a059618,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_56, 0xe2801207,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_60, 0x75b227eb,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_64, 0x1a2c8309,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_68, 0xa05a6e1b,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_72, 0xb3d63b52,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_76, 0x842fe329,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_80, 0xed00d153,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_84, 0x5bb1fc20,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_88, 0x39becb6a,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_92, 0xcf584c4a,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_96, 0xfbaaefd0,  \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_100, 0x85334d43, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_104, 0x7f02f945, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_108, 0xa89f3c50, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_112, 0x8f40a351, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_116, 0xf5389d92, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_120, 0x21dab6bc, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_124, 0xd2f3ff10, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_128, 0xec130ccd, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_132, 0x1744975f, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_136, 0x3d7ea7c4, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_140, 0x73195d64, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_144, 0xdc4f8160, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_148, 0x88902a22, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_152, 0x14b8ee46, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_156, 0xdb0b5ede, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_160, 0x0a3a32e0, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_164, 0x5c240649, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_168, 0x62acd3c2, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_172, 0x79e49591, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_176, 0x6d37c8e7, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_180, 0xa94ed58d, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_184, 0xeaf4566c, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_188, 0x08ae7a65, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_192, 0x2e2578ba, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_196, 0xc6b4a61c, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_200, 0x1f74dde8, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_204, 0x8a8bbd4b, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_208, 0x66b53e70, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_212, 0x0ef60348, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_216, 0xb9573561, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_220, 0x9e1dc186, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_224, 0x1198f8e1, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_228, 0x948ed969, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_232, 0xe9871e9b, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_236, 0xdf2855ce, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_240, 0x0d89a18c, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_244, 0x6842e6bf, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_248, 0x0f2d9941, \
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_SBOX_252, 0x16bb54b0

// Calculate 4 AES tables
// using 8 local threads
// Requirements:
//  - types TYPE_UINT, TYPE_PTR_FN_UINT, TYPE_PTR_WG_UINT
//  - local size must be set to [8,1,1]
//  - declared variables: AES_0, AES_1, AES_2, AES_3 as type: [workgroup] uint[256]
//  - built-in variables LOCAL_SIZE_X, LOCAL_INVOCATION_X should be in scope
//  - constants: CONST_UINT_1, CONST_UINT_2, CONST_UINT_3, CONST_UINT_7, CONST_UINT_8,
//               CONST_UINT_16, CONST_UINT_24, CONST_UINT_256, CONST_UINT_AES_WPOLY=0x11b
//  - PTR_CONST_AES_SBOX0: pointer to const uint[64]

#define aes_gen_tables_enum          \
  AES_GEN_TABLES__ROTL_SL_1,         \
  AES_GEN_TABLES__ROTL_SR_1,         \
  AES_GEN_TABLES__C_1,               \
  AES_GEN_TABLES__PTR_AES_1_I,       \
  AES_GEN_TABLES__ROTL_SL_2,         \
  AES_GEN_TABLES__ROTL_SR_2,         \
  AES_GEN_TABLES__C_2,               \
  AES_GEN_TABLES__PTR_AES_2_I,       \
  AES_GEN_TABLES__ROTL_SL_3,         \
  AES_GEN_TABLES__ROTL_SR_3,         \
  AES_GEN_TABLES__C_3,               \
  AES_GEN_TABLES__PTR_AES_3_I,       \
  AES_GEN_TABLES__LABEL_BEGIN,       \
  AES_GEN_TABLES__LABEL_LOOP,        \
  AES_GEN_TABLES__I,                 \
  AES_GEN_TABLES__I_INC,             \
  AES_GEN_TABLES__LABEL_LOOP_COND,   \
  AES_GEN_TABLES__LABEL_LOOP_BODY,   \
  AES_GEN_TABLES__LABEL_LOOP_END,    \
  AES_GEN_TABLES__SBOX0_PACKED_GIDX, \
  AES_GEN_TABLES__PTR_SBOX0_I,       \
  AES_GEN_TABLES__SBOX0_I,           \
  AES_GEN_TABLES__SBOX0_PACKED_LIDX, \
  AES_GEN_TABLES__SBOX0_PACKED_OFF,  \
  AES_GEN_TABLES__X,                 \
  AES_GEN_TABLES__X_SL_1,            \
  AES_GEN_TABLES__X_BIT7,            \
  AES_GEN_TABLES__X_MUL_WPOLY,       \
  AES_GEN_TABLES__X_A,               \
  AES_GEN_TABLES__X_B,               \
  AES_GEN_TABLES__C_8_16,            \
  AES_GEN_TABLES__C_8_16_24,         \
  AES_GEN_TABLES__C,                 \
  AES_GEN_TABLES__PTR_AES_0_I

#define aes_gen_tables_const0_rotate(c,nl,nr)                                    \
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, AES_GEN_TABLES__ROTL_SL_##c,     \
              AES_GEN_TABLES__C, CONST_UINT_##nl,                                \
  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_UINT, AES_GEN_TABLES__ROTL_SR_##c,    \
              AES_GEN_TABLES__C, CONST_UINT_##nr,                                \
  (5 << 16) | OP_BITWISE_OR, TYPE_UINT, AES_GEN_TABLES__C_##c,                   \
              AES_GEN_TABLES__ROTL_SL_##c, AES_GEN_TABLES__ROTL_SR_##c,          \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, AES_GEN_TABLES__PTR_AES_##c##_I,\
              AES_##c, AES_GEN_TABLES__I,                                        \
  (3 << 16) | OP_STORE, AES_GEN_TABLES__PTR_AES_##c##_I, AES_GEN_TABLES__C_##c

#define aes_gen_tables                                                                         \
  /* for (uint i = get_local_id(0); i < 256; i += get_local_size(0)) */                        \
  (2 << 16) | OP_BRANCH, AES_GEN_TABLES__LABEL_BEGIN,                                          \
  (2 << 16) | OP_LABEL, AES_GEN_TABLES__LABEL_BEGIN,                                           \
  (2 << 16) | OP_BRANCH, AES_GEN_TABLES__LABEL_LOOP,                                           \
  (2 << 16) | OP_LABEL, AES_GEN_TABLES__LABEL_LOOP,                                            \
  (7 << 16) | OP_PHI, TYPE_UINT, AES_GEN_TABLES__I,                                            \
              LOCAL_INVOCATION_X, AES_GEN_TABLES__LABEL_BEGIN,                                 \
              AES_GEN_TABLES__I_INC, AES_GEN_TABLES__LABEL_LOOP_BODY,                          \
  (5 << 16) | OP_ULESS_THAN, TYPE_BOOL, AES_GEN_TABLES__LABEL_LOOP_COND,                       \
              AES_GEN_TABLES__I, CONST_UINT_256,                                               \
  (3 << 16) | OP_LOOP_MERGE, AES_GEN_TABLES__LABEL_LOOP_END, AES_GEN_TABLES__LABEL_LOOP_BODY,  \
  (4 << 16) | OP_BRANCH_CONDITIONAL, AES_GEN_TABLES__LABEL_LOOP_COND,                          \
              AES_GEN_TABLES__LABEL_LOOP_BODY, AES_GEN_TABLES__LABEL_LOOP_END,                 \
  (2 << 16) | OP_LABEL, AES_GEN_TABLES__LABEL_LOOP_BODY,                                       \
  /* unpack AES_SBOX0 const */                                                                 \
  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_UINT, AES_GEN_TABLES__SBOX0_PACKED_GIDX,            \
              AES_GEN_TABLES__I, CONST_UINT_2,                                                 \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_FN_UINT, AES_GEN_TABLES__PTR_SBOX0_I,                  \
              PTR_CONST_AES_SBOX0, AES_GEN_TABLES__SBOX0_PACKED_GIDX,                          \
  (4 << 16) | OP_LOAD, TYPE_UINT, AES_GEN_TABLES__SBOX0_I, AES_GEN_TABLES__PTR_SBOX0_I,        \
  (5 << 16) | OP_BITWISE_AND, TYPE_UINT, AES_GEN_TABLES__SBOX0_PACKED_LIDX,                    \
              AES_GEN_TABLES__I, CONST_UINT_3,                                                 \
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, AES_GEN_TABLES__SBOX0_PACKED_OFF,              \
              AES_GEN_TABLES__SBOX0_PACKED_LIDX, CONST_UINT_3,                                 \
  (6 << 16) | OP_BITFIELD_UEXTRACT, TYPE_UINT, AES_GEN_TABLES__X, AES_GEN_TABLES__SBOX0_I,     \
              AES_GEN_TABLES__SBOX0_PACKED_OFF, CONST_UINT_8,                                  \
  /* x = AES_SBOX0[i] */                                                                       \
  /* a = (x << 1) ^ (bitextract(x, 8, 1) * AES_WPOLY) */                                       \
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, AES_GEN_TABLES__X_SL_1,                        \
              AES_GEN_TABLES__X, CONST_UINT_1,                                                 \
  (6 << 16) | OP_BITFIELD_UEXTRACT, TYPE_UINT, AES_GEN_TABLES__X_BIT7,                         \
              AES_GEN_TABLES__X, CONST_UINT_7, CONST_UINT_1,                                   \
  (5 << 16) | OP_IMUL, TYPE_UINT, AES_GEN_TABLES__X_MUL_WPOLY,                                 \
              AES_GEN_TABLES__X_BIT7, CONST_UINT_AES_WPOLY,                                    \
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT, AES_GEN_TABLES__X_A,                                  \
              AES_GEN_TABLES__X_SL_1, AES_GEN_TABLES__X_MUL_WPOLY,                             \
  /* b = a ^ x */                                                                              \
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT, AES_GEN_TABLES__X_B,                                  \
              AES_GEN_TABLES__X_A, AES_GEN_TABLES__X,                                          \
  /* AES_0[i] = uint(a,x,x,b) */                                                               \
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, AES_GEN_TABLES__C_8_16, AES_GEN_TABLES__X_A,      \
              AES_GEN_TABLES__X, CONST_UINT_8, CONST_UINT_8,                                   \
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, AES_GEN_TABLES__C_8_16_24, AES_GEN_TABLES__C_8_16,\
              AES_GEN_TABLES__X, CONST_UINT_16, CONST_UINT_8,                                  \
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, AES_GEN_TABLES__C, AES_GEN_TABLES__C_8_16_24,     \
              AES_GEN_TABLES__X_B, CONST_UINT_24, CONST_UINT_8,                                \
                                                                                               \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, AES_GEN_TABLES__PTR_AES_0_I,                  \
              AES_0, AES_GEN_TABLES__I,                                                        \
  (3 << 16) | OP_STORE, AES_GEN_TABLES__PTR_AES_0_I, AES_GEN_TABLES__C,                        \
                                                                                               \
  aes_gen_tables_const0_rotate(1,8,24),                                                        \
  aes_gen_tables_const0_rotate(2,16,16),                                                       \
  aes_gen_tables_const0_rotate(3,24,8),                                                        \
                                                                                               \
  (5 << 16) | OP_IADD, TYPE_UINT, AES_GEN_TABLES__I_INC,                                       \
              AES_GEN_TABLES__I, LOCAL_SIZE_X, /* ; i += local_size */                         \
  (2 << 16) | OP_BRANCH, AES_GEN_TABLES__LABEL_LOOP,                                           \
  (2 << 16) | OP_LABEL, AES_GEN_TABLES__LABEL_LOOP_END

// clang-format on
