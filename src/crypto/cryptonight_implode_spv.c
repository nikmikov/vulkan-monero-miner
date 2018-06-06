#include "utils/spirv.h"

#include "crypto/cryptonight_spv.h"

#include "crypto/aes_spv.h"

enum { // variables
  RESERVED_ID = 0,
  EXT_INST_GLSL_STD_450,
  FUNC_MAIN,
  LABEL_MAIN,
  // built-in global variables
  WORKGROUP_ID,
  LOCAL_INVOCATION_ID,
  WORKGROUP_X,
  LOCAL_INVOCATION_X,
  // types
  TYPE_VOID,
  TYPE_FUNC_VOID,
  TYPE_FUNC_UINT_UINT_UINT,
  TYPE_FUNC_UINT_UINT,
  TYPE_FUNC_UINT4_UINT4_UINT_UINT_UINT_UINT,
  TYPE_FUNC_VOID_UINT4,
  TYPE_BOOL,
  TYPE_UINT,
  TYPE_UINT3,
  TYPE_UINT4,
  TYPE_ARRAY_UINT_40,
  TYPE_ARRAY_UINT_50,
  TYPE_CONST_ARRAY_UINT_64,
  TYPE_ARRAY_UINT_256,
  TYPE_RT_ARRAY_ARRAY_UINT_50,
  TYPE_ARRAY_UINT4_131072,
  TYPE_RT_ARRAY_ARRAY_UINT4_131072,
  TYPE_STRUCT_STATE_BUFFER,
  TYPE_STRUCT_SCRATCHPAD_BUFFER,
  // pointer types
  TYPE_PTR_WG_UINT,
  TYPE_PTR_WG_ARRAY_UINT_40,
  TYPE_PTR_WG_ARRAY_UINT_256,
  TYPE_PTR_IN_UINT,
  TYPE_PTR_IN_UINT3,
  TYPE_PTR_FN_CONST_ARRAY_UINT_64,
  TYPE_PTR_FN_UINT,
  TYPE_PTR_FN_UINT4,
  TYPE_PTR_BF_UINT,
  TYPE_PTR_BF_UINT4,
  TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT_50,
  TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT4_131072,
  TYPE_PTR_BF_STATE_BUFFER,
  TYPE_PTR_BF_ARRAY_UINT_50,
  TYPE_PTR_BF_SCRATCHPAD_BUFFER,
  TYPE_PTR_BF_ARRAY_UINT4_131072,

  // pointers
  PTR_WORKGROUP_X,
  PTR_LOCAL_INVOCATION_X,
  PTR_STATE_BUFFER,
  PTR_STATE_BUFFER_INV,
  PTR_SCRATCHPAD_BUFFER,
  PTR_SCRATCHPAD_BUFFER_INV,
  PTR_HASH_STATE,
  PTR_SCRATCHPAD,

  // main: local variables
  LOCAL_INVOCATION_X_PLUS_8,
  PTR_HASH_STATE_LINV,
  HASH_STATE_LINV,
  PTR_AES_KEY_LINV,
  LOCAL_INVOCATION_X_PLUS_4,
  PTR_XOUT,
  VAL_XOUT_0,
  VAL_XOUT_FINAL,

#define main_local_vars(n) VAL_BASE_IDX_##n, PTR_HASH_STATE_##n, HASH_STATE_##n, VAL_XOUT_FINAL_##n
  main_local_vars(0),
  main_local_vars(1),
  main_local_vars(2),
  main_local_vars(3),

  LABEL_MAIN_BLOCK,
  LABEL_LOOP_MAIN,
  LABEL_LOOP_MAIN_BODY,
  LABEL_LOOP_MAIN_END,
  VAL_LOOP_MAIN_COND,
  PTR_SCRATCHPAD_XIN,
  VAL_LOOP_MAIN_I,
  VAL_LOOP_MAIN_INC,
  PTR_SCRATCHPAD_I,
  SCRATCHPAD_I,
  VAL_XOUT,
  XOUT_XOR_SCRATCHPAD_I,
  VAL_AES_ENCODE_10,

  // constants
  CONST_UINT_0,
  CONST_UINT_1,
  CONST_UINT_2,
  CONST_UINT_3,
  CONST_UINT_4,
  CONST_UINT_7,
  CONST_UINT_8,
  CONST_UINT_16,
  CONST_UINT_24,
  CONST_UINT_32,
  CONST_UINT_40,
  CONST_UINT_50,
  CONST_UINT_64,
  CONST_UINT_0xFF,
  CONST_UINT_256,
  CONST_UINT_131072,
  CONST_UINT_AES_WPOLY,
  // AES tables calculation
  CONST_AES_SBOX0,
  PTR_CONST_AES_SBOX0,
  AES_0, AES_1, AES_2, AES_3,
  // from aes_spv.h
  aes_sbox_const_enum,
  aes_gen_tables_enum,
  aes_encode_enum,
  aes_encode_10_enum,
  aes_expand_key_10_enum,
  // aes gen key loop
  AES_KEY,


  BOUND
};

// clang-format off
const uint32_t cryptonight_implode_shader[] = {
  // HEADER
  SPIRV_MAGIC,
  0x00010300, // version 1.3.0
  0,          // generator (optional)
  BOUND,      // bound
  0,          // schema
  (2 << 16) | OP_CAPABILITY, CAP_SHADER,
  (11 << 16)| OP_EXTENSION, LIT_SPV_KHR_storage_buffer_storage_class,
  (6 << 16) | OP_EXT_INST_IMPORT, EXT_INST_GLSL_STD_450, LIT_GLSL_std_450,
  (3 << 16) | OP_MEMORY_MODEL, AM_LOGICAL, MM_GLSL450,
  (5 << 16) | OP_ENTRY_POINT, EXEC_MODEL_GLCOMPUTE, FUNC_MAIN, LIT_MAIN,
  (6 << 16) | OP_EXECUTION_MODE, FUNC_MAIN, EXEC_MODE_LOCALSIZE, 8, 1, 1,
  // DECORATIONS
  (4 << 16) | OP_DECORATE, WORKGROUP_ID, DECOR_BUILTIN, BUILTIN_WORKGROUP_ID,
  (4 << 16) | OP_DECORATE, LOCAL_INVOCATION_ID, DECOR_BUILTIN, BUILTIN_LOCAL_INVOCATION_ID,
  (4 << 16) | OP_DECORATE, TYPE_ARRAY_UINT_40, DECOR_ARRAY_STRIDE, 4,
  (4 << 16) | OP_DECORATE, TYPE_ARRAY_UINT_50, DECOR_ARRAY_STRIDE, 4,
  (4 << 16) | OP_DECORATE, TYPE_ARRAY_UINT_256, DECOR_ARRAY_STRIDE, 4,
  (4 << 16) | OP_DECORATE, TYPE_ARRAY_UINT4_131072, DECOR_ARRAY_STRIDE, 16,
  // state buffer
  (4 << 16) | OP_DECORATE, TYPE_RT_ARRAY_ARRAY_UINT_50, DECOR_ARRAY_STRIDE, 200,
  (3 << 16) | OP_DECORATE, TYPE_STRUCT_STATE_BUFFER, DECOR_BLOCK,
  (4 << 16) | OP_DECORATE, PTR_STATE_BUFFER, DECOR_DESCRIPTOR_SET, 0,
  (4 << 16) | OP_DECORATE, PTR_STATE_BUFFER, DECOR_BINDING, 0,
  (5 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_STATE_BUFFER, 0, DECOR_OFFSET, 0,
  // scratchpad buffer
  (4 << 16) | OP_DECORATE, TYPE_RT_ARRAY_ARRAY_UINT4_131072, DECOR_ARRAY_STRIDE, 2097152,
  (3 << 16) | OP_DECORATE, TYPE_STRUCT_SCRATCHPAD_BUFFER, DECOR_BLOCK,
  (4 << 16) | OP_DECORATE, PTR_SCRATCHPAD_BUFFER, DECOR_DESCRIPTOR_SET, 0,
  (4 << 16) | OP_DECORATE, PTR_SCRATCHPAD_BUFFER, DECOR_BINDING, 1,
  (5 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_SCRATCHPAD_BUFFER, 0, DECOR_OFFSET, 0,

  // SCALAR AND VECTOR TYPES
  (2 << 16) | OP_TYPE_VOID, TYPE_VOID,                    //type: void
  (2 << 16) | OP_TYPE_BOOL, TYPE_BOOL,                    //type: bool
  (4 << 16) | OP_TYPE_INT, TYPE_UINT, 32, 0,              //type: uint
  (4 << 16) | OP_TYPE_VECTOR, TYPE_UINT3, TYPE_UINT, 3,   //type: uvec3
  (4 << 16) | OP_TYPE_VECTOR, TYPE_UINT4, TYPE_UINT, 4,   //type: uvec4

  // CONST scalar
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_0, 0, // 0U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_1, 1, // 1U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_2, 2, // 2U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_3, 3, // 3U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_4, 4, // 4U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_7, 7, // 7U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_8, 8, // 8U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_16, 16, // 16U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_24, 24, // 24U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_32, 32, // 32U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_40, 40, // 40U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_50, 50, // 50U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_64, 64, // 64U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_0xFF, 0xff, // 0xffU
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_256, 256, // 256U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_131072, 131072, // 131072U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_WPOLY, 0x011b, // 0x011b
  aes_sbox_const, /** uint8_t[256] SBOX const packed into uint[64]*/

  // ARRAY types
  (4 << 16) | OP_TYPE_ARRAY, TYPE_ARRAY_UINT_40, TYPE_UINT, CONST_UINT_40, //type: uint[40]
  (4 << 16) | OP_TYPE_ARRAY, TYPE_ARRAY_UINT_50, TYPE_UINT, CONST_UINT_50, //type: uint[50]
  (4 << 16) | OP_TYPE_ARRAY, TYPE_CONST_ARRAY_UINT_64, TYPE_UINT, CONST_UINT_64, //type: uint[64]
  (4 << 16) | OP_TYPE_ARRAY, TYPE_ARRAY_UINT_256, TYPE_UINT, CONST_UINT_256, //type: uint[256]
  (4 << 16) | OP_TYPE_ARRAY, TYPE_ARRAY_UINT4_131072, TYPE_UINT4, CONST_UINT_131072, //type: uint4[131072]
  (3 << 16) | OP_TYPE_RUNTIME_ARRAY, TYPE_RT_ARRAY_ARRAY_UINT_50, TYPE_ARRAY_UINT_50,
  (3 << 16) | OP_TYPE_RUNTIME_ARRAY, TYPE_RT_ARRAY_ARRAY_UINT4_131072, TYPE_ARRAY_UINT4_131072,

  // CONST composite
  (67 << 16)| OP_CONSTANT_COMPOSITE, TYPE_CONST_ARRAY_UINT_64, CONST_AES_SBOX0, aes_sbox_const_enum,

  // STRUCT types
  (3 << 16) | OP_TYPE_STRUCT, TYPE_STRUCT_STATE_BUFFER, TYPE_RT_ARRAY_ARRAY_UINT_50,
  (3 << 16) | OP_TYPE_STRUCT, TYPE_STRUCT_SCRATCHPAD_BUFFER, TYPE_RT_ARRAY_ARRAY_UINT4_131072,

  // POINTER TYPES
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_WG_UINT, SC_WORKGROUP, TYPE_UINT,   //type: [Workgroup] uint*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_WG_ARRAY_UINT_40, SC_WORKGROUP, TYPE_ARRAY_UINT_40,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_WG_ARRAY_UINT_256, SC_WORKGROUP, TYPE_ARRAY_UINT_256,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_IN_UINT, SC_INPUT, TYPE_UINT,   //type: [Input] uint*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_IN_UINT3, SC_INPUT, TYPE_UINT3, //type: [Input] uint3*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_FN_CONST_ARRAY_UINT_64, SC_FUNCTION, TYPE_CONST_ARRAY_UINT_64,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_FN_UINT, SC_FUNCTION, TYPE_UINT,   //type: [Function] uint*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_FN_UINT4, SC_FUNCTION, TYPE_UINT4,   //type: [Function] uint4*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_UINT, SC_BUFFER, TYPE_UINT,   //type: [Buffer] uint*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT_50, SC_BUFFER, TYPE_RT_ARRAY_ARRAY_UINT_50,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT4_131072, SC_BUFFER, TYPE_RT_ARRAY_ARRAY_UINT4_131072,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_STATE_BUFFER, SC_BUFFER, TYPE_STRUCT_STATE_BUFFER,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_ARRAY_UINT_50, SC_BUFFER, TYPE_ARRAY_UINT_50,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_SCRATCHPAD_BUFFER, SC_BUFFER, TYPE_STRUCT_SCRATCHPAD_BUFFER,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_ARRAY_UINT4_131072, SC_BUFFER, TYPE_ARRAY_UINT4_131072,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_UINT4, SC_BUFFER, TYPE_UINT4,
  // FUNCTION TYPES
  (3 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_VOID, TYPE_VOID,//type: void fn()
  (4 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_VOID_UINT4, TYPE_VOID, TYPE_PTR_FN_UINT4, //type: uint4 fn(uint4*)
  (4 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_UINT_UINT, TYPE_UINT, TYPE_UINT,  //type: uint fn(uint)
  (5 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_UINT_UINT_UINT, TYPE_UINT, TYPE_UINT, TYPE_UINT,  //type: uint fn(uint,uint)
  (8 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_UINT4_UINT4_UINT_UINT_UINT_UINT,
              TYPE_UINT4, TYPE_PTR_FN_UINT4, TYPE_UINT, TYPE_UINT, TYPE_UINT, TYPE_UINT, //type: uint4 fn(uint4*,uint,uint,uint,uint)

  // GLOBAL VARIABLES
  (4 << 16) | OP_VARIABLE, TYPE_PTR_IN_UINT3, WORKGROUP_ID, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_IN_UINT3, LOCAL_INVOCATION_ID, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_BF_STATE_BUFFER, PTR_STATE_BUFFER, SC_BUFFER,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_BF_SCRATCHPAD_BUFFER, PTR_SCRATCHPAD_BUFFER, SC_BUFFER,

  // Local(shared) AES tables and 10 rounds key
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_0, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_1, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_2, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_3, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_40, AES_KEY, SC_WORKGROUP, /** 10 rounds key */

  aes_encode_fn,
  aes_encode_10_fn,

  // MAIN
  (5 << 16) | OP_FUNCTION, TYPE_VOID, FUNC_MAIN, FNC_NONE, TYPE_FUNC_VOID,
  (2 << 16) | OP_LABEL, LABEL_MAIN,
  // variables
  (4 << 16) | OP_VARIABLE, TYPE_PTR_FN_UINT4, PTR_XOUT, SC_FUNCTION,
  (5 << 16) | OP_VARIABLE, TYPE_PTR_FN_CONST_ARRAY_UINT_64, PTR_CONST_AES_SBOX0, SC_FUNCTION, CONST_AES_SBOX0,
  // get global invocation index
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_IN_UINT, PTR_WORKGROUP_X, WORKGROUP_ID, CONST_UINT_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, WORKGROUP_X, PTR_WORKGROUP_X,
  // get local invocation index
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_IN_UINT, PTR_LOCAL_INVOCATION_X, LOCAL_INVOCATION_ID, CONST_UINT_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, LOCAL_INVOCATION_X, PTR_LOCAL_INVOCATION_X,
  // get pointer to HASH_STATE array for current invocation
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT_50, PTR_STATE_BUFFER_INV, PTR_STATE_BUFFER, CONST_UINT_0,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_ARRAY_UINT_50, PTR_HASH_STATE, PTR_STATE_BUFFER_INV, WORKGROUP_X,
  // get pointer to SCRATCHPAD array for current invocation
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT4_131072, PTR_SCRATCHPAD_BUFFER_INV, PTR_SCRATCHPAD_BUFFER, CONST_UINT_0,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_ARRAY_UINT4_131072, PTR_SCRATCHPAD, PTR_SCRATCHPAD_BUFFER_INV, WORKGROUP_X,
  // copy bytes 32..63 of state to aes_key array
  (5 << 16) | OP_IADD, TYPE_UINT, LOCAL_INVOCATION_X_PLUS_8, LOCAL_INVOCATION_X, CONST_UINT_8,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_HASH_STATE_LINV, PTR_HASH_STATE, LOCAL_INVOCATION_X_PLUS_8,
  (4 << 16) | OP_LOAD, TYPE_UINT, HASH_STATE_LINV, PTR_HASH_STATE_LINV,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_AES_KEY_LINV, AES_KEY, LOCAL_INVOCATION_X,
  (3 << 16) | OP_STORE, PTR_AES_KEY_LINV, HASH_STATE_LINV,

  // calculate aes0,aes1,aes2,aes3 tables
  aes_gen_tables,
  (4 << 16) | OP_CONTROL_BARRIER, CONST_UINT_2, CONST_UINT_2, CONST_UINT_256,
  (2 << 16) | OP_BRANCH, AES_EXPAND_KEY_10__LABEL,
  aes_expand_key_10,
  (4 << 16) | OP_CONTROL_BARRIER, CONST_UINT_2, CONST_UINT_2, CONST_UINT_256,
  (2 << 16) | OP_BRANCH, LABEL_MAIN_BLOCK,
  (2 << 16) | OP_LABEL, LABEL_MAIN_BLOCK,
  // The bytes 64..191 are extracted from the Keccak final state and split into 8 blocks of 16 bytes each.
  // 1 block per local thread
  // base_idx = 16 + local_idx * 4 => 4 * (local_idx + 4)
  (5 << 16) | OP_IADD, TYPE_UINT, LOCAL_INVOCATION_X_PLUS_4, LOCAL_INVOCATION_X, CONST_UINT_4,
  (5 << 16) | OP_IMUL, TYPE_UINT, VAL_BASE_IDX_0, LOCAL_INVOCATION_X_PLUS_4, CONST_UINT_4,

  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_HASH_STATE_0, PTR_HASH_STATE, VAL_BASE_IDX_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, HASH_STATE_0, PTR_HASH_STATE_0,

#define load_state_word(c)                                            \
  (5 << 16) | OP_IADD, TYPE_UINT, VAL_BASE_IDX_##c, VAL_BASE_IDX_0, CONST_UINT_##c, \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_HASH_STATE_##c, PTR_HASH_STATE, VAL_BASE_IDX_##c, \
  (4 << 16) | OP_LOAD, TYPE_UINT, HASH_STATE_##c, PTR_HASH_STATE_##c

  load_state_word(1),
  load_state_word(2),
  load_state_word(3),

  (7 << 16) | OP_COMPOSITE_CONSTRUCT, TYPE_UINT4, VAL_XOUT_0, HASH_STATE_0, HASH_STATE_1, HASH_STATE_2, HASH_STATE_3,
  (3 << 16) | OP_STORE, PTR_XOUT, VAL_XOUT_0,

  //  for (uint i = local_id(0); i < CRYPTONIGHT_MEMORY_UINT; i += 8) {
  (2 << 16) | OP_BRANCH, LABEL_LOOP_MAIN,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN,
  (7 << 16) | OP_PHI, TYPE_UINT, VAL_LOOP_MAIN_I, LOCAL_INVOCATION_X, LABEL_MAIN_BLOCK,
              VAL_LOOP_MAIN_INC, LABEL_LOOP_MAIN_BODY,

  (5 << 16) | OP_ULESS_THAN, TYPE_BOOL, VAL_LOOP_MAIN_COND,  VAL_LOOP_MAIN_I, CONST_UINT_131072, // i < 131072 ?
  (4 << 16) | OP_LOOP_MERGE, LABEL_LOOP_MAIN_END, LABEL_LOOP_MAIN_BODY, LC_NONE,
  (4 << 16) | OP_BRANCH_CONDITIONAL, VAL_LOOP_MAIN_COND, LABEL_LOOP_MAIN_BODY, LABEL_LOOP_MAIN_END,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN_BODY,

  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT4, PTR_SCRATCHPAD_I, PTR_SCRATCHPAD, VAL_LOOP_MAIN_I,
  (4 << 16) | OP_LOAD, TYPE_UINT4, SCRATCHPAD_I, PTR_SCRATCHPAD_I,
  (4 << 16) | OP_LOAD, TYPE_UINT4, VAL_XOUT, PTR_XOUT,
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT4, XOUT_XOR_SCRATCHPAD_I, VAL_XOUT, SCRATCHPAD_I,
  (3 << 16) | OP_STORE, PTR_XOUT, XOUT_XOR_SCRATCHPAD_I,
  (5 << 16) | OP_FUNCTION_CALL, TYPE_VOID, VAL_AES_ENCODE_10, FUNC_AES_ENCODE_10, PTR_XOUT,

  (5 << 16) | OP_IADD, TYPE_UINT, VAL_LOOP_MAIN_INC, VAL_LOOP_MAIN_I, CONST_UINT_8,
  (2 << 16) | OP_BRANCH, LABEL_LOOP_MAIN,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN_END,
  // store xout in the hash state
  (4 << 16) | OP_LOAD, TYPE_UINT4, VAL_XOUT_FINAL, PTR_XOUT,
#define store_final(n)                                                                \
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_XOUT_FINAL_##n, VAL_XOUT_FINAL, n, \
  (3 << 16) | OP_STORE, PTR_HASH_STATE_##n, VAL_XOUT_FINAL_##n

  store_final(0),
  store_final(1),
  store_final(2),
  store_final(3),

  // main:return
  (1 << 16) | OP_RETURN,
  (1 << 16) | OP_FUNCTION_END,

};

// clang-format on

const size_t cryptonight_implode_shader_size =
    sizeof(cryptonight_implode_shader);
