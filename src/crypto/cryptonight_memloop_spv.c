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
  TYPE_FUNC_UINT4_UINT4_UINT_UINT_UINT_UINT,
  TYPE_BOOL,
  TYPE_UINT,
  TYPE_ULONG,
  TYPE_UINT2,
  TYPE_UINT3,
  TYPE_UINT4,
  TYPE_ULONG2,
  TYPE_ARRAY_UINT_50,
  TYPE_CONST_ARRAY_UINT_64,
  TYPE_ARRAY_UINT_256,
  TYPE_RT_ARRAY_ARRAY_UINT_50,
  TYPE_ARRAY_UINT4_131072,
  TYPE_RT_ARRAY_ARRAY_UINT4_131072,
  TYPE_STRUCT_STATE_BUFFER,
  TYPE_STRUCT_SCRATCHPAD_BUFFER,
  TYPE_STRUCT_MUL_ULONG,
  // pointer types
  TYPE_PTR_WG_UINT,
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

  // constants
  CONST_UINT_0,
  CONST_UINT_1,
  CONST_UINT_2,
  CONST_UINT_3,
  CONST_UINT_4,
  CONST_UINT_5,
  CONST_UINT_6,
  CONST_UINT_7,
  CONST_UINT_8,
  CONST_UINT_9,
  CONST_UINT_10,
  CONST_UINT_11,
  CONST_UINT_12,
  CONST_UINT_13,
  CONST_UINT_14,
  CONST_UINT_15,
  CONST_UINT_16,
  CONST_UINT_24,
  CONST_UINT_50,
  CONST_UINT_64,
  CONST_UINT_256,
  CONST_UINT_0xFFF,
  CONST_UINT_131072,
  CONST_UINT_0x80000,
  CONST_UINT_AES_WPOLY,
  // AES tables calculation
  CONST_AES_SBOX0,
  PTR_CONST_AES_SBOX0,
  AES_0, AES_1, AES_2, AES_3,

  // from aes_spv.h
  aes_sbox_const_enum,
  aes_gen_tables_enum,
  aes_encode_enum,
  // to_scratchpad_idx
  FUNC_TO_SCRATCHPAD_IDX,
  TO_SCRATCHPAD_IDX__ARG_HI,
  TO_SCRATCHPAD_IDX__ARG_LO,
  TO_SCRATCHPAD_IDX__LABEL,
  TO_SCRATCHPAD_IDX__X,
  TO_SCRATCHPAD_IDX__X_AND_0xFFF,
  TO_SCRATCHPAD_IDX__RESULT,


#define hash_state(n) PTR_HASH_STATE_##n, HASH_STATE_##n

  hash_state(0),
  hash_state(1),
  hash_state(2),
  hash_state(3),
  hash_state(4),
  hash_state(5),
  hash_state(6),
  hash_state(7),
  hash_state(8),
  hash_state(9),
  hash_state(10),
  hash_state(11),
  hash_state(12),
  hash_state(13),
  hash_state(14),
  hash_state(15),
  HASH_STATE_V0,
  HASH_STATE_V1,
  HASH_STATE_V2,
  HASH_STATE_V3,
  VAL_A0, VAL_B0,
  VAL_A, VAL_B,
  VAL_A_0, VAL_A_1, VAL_A_2, VAL_A_3,
  VAL_IDX_A, VAL_IDX_B,
  VAL_AX, VAL_BX, VAL_CX,
  PTR_SCRATCHPAD_0, PTR_SCRATCHPAD_1,
  PTR_SCRATCHPAD_IDX_A,
  VAL_SCRATCHPAD_IDX_A,
  PTR_SCRATCHPAD_IDX_B,
  VAL_SCRATCHPAD_IDX_B,
  VAL_SCRATCHPAD_IDX_B_AS_ULONG2,
  VAR_AES_ENCODE_IN,
  VAL_A_UPD,
  VAL_B_UPD,
  VAL_B_UPD_XOR_B,
  VAL_B_UPD_AS_ULONG2,
  VAL_B_UPD_0,
  VAL_B_UPD_1,
  VAL_CX_0,
  VAL_CX_1,
  VAL_CX_SWAPPED,
  VAL_DX,
  VAL_DX_ULONG2,
  VAL_DX_ULONG2_SR16,
  VAL_DX_A, VAL_DX_B,
  VAL_DX_0, VAL_DX_1, VAL_DX_2, VAL_DX_3,
  VAL_A_AS_ULONG2,
  LABEL_LOOP_MAIN_START,
  LABEL_LOOP_MAIN,
  LABEL_LOOP_MAIN_BODY,
  LABEL_LOOP_MAIN_END,
  VAL_LOOP_MAIN_COND,
  VAL_LOOP_MAIN_I,
  VAL_LOOP_MAIN_I_INC,
  XXX,
  BOUND
};

// clang-format off
const uint32_t cryptonight_memloop_shader[] = {
  // HEADER
  SPIRV_MAGIC,
  0x00010300, // version 1.3.0
  0,          // generator (optional)
  BOUND,      // bound
  0,          // schema
  (2 << 16) | OP_CAPABILITY, CAP_SHADER,
  (2 << 16) | OP_CAPABILITY, CAP_INT64,
  (11 << 16)| OP_EXTENSION, LIT_SPV_KHR_storage_buffer_storage_class,
  (6 << 16) | OP_EXT_INST_IMPORT, EXT_INST_GLSL_STD_450, LIT_GLSL_std_450,
  (3 << 16) | OP_MEMORY_MODEL, AM_LOGICAL, MM_GLSL450,
  (5 << 16) | OP_ENTRY_POINT, EXEC_MODEL_GLCOMPUTE, FUNC_MAIN, LIT_MAIN,
  (6 << 16) | OP_EXECUTION_MODE, FUNC_MAIN, EXEC_MODE_LOCALSIZE, 1, 1, 1,
  // DECORATIONS
  (4 << 16) | OP_DECORATE, WORKGROUP_ID, DECOR_BUILTIN, BUILTIN_WORKGROUP_ID,
  (4 << 16) | OP_DECORATE, LOCAL_INVOCATION_ID, DECOR_BUILTIN, BUILTIN_LOCAL_INVOCATION_ID,
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
  (4 << 16) | OP_TYPE_INT, TYPE_ULONG, 64, 0,            //type: ulong
  (4 << 16) | OP_TYPE_VECTOR, TYPE_UINT2, TYPE_UINT, 2,   //type: uvec2
  (4 << 16) | OP_TYPE_VECTOR, TYPE_UINT3, TYPE_UINT, 3,   //type: uvec3
  (4 << 16) | OP_TYPE_VECTOR, TYPE_UINT4, TYPE_UINT, 4,   //type: uvec4
  (4 << 16) | OP_TYPE_VECTOR, TYPE_ULONG2, TYPE_ULONG, 2,   //type: ulong2

  // CONST scalar
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_0, 0, // 0U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_1, 1, // 1U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_2, 2, // 2U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_3, 3, // 3U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_4, 4, // 4U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_5, 5, // 5U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_6, 6, // 6U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_7, 7, // 7U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_8, 8, // 8U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_9, 9, // 9U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_10, 10, // 10U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_11, 11, // 11U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_12, 12, // 12U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_13, 13, // 13U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_14, 14, // 14U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_15, 15, // 15U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_16, 16, // 16U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_24, 24, // 24U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_50, 50, // 50U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_64, 64, // 64U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_256, 256, // 256U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_0xFFF, 0xfff, // 0xFFFU
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_131072, 131072, // 131072U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_0x80000, 0x80000, // 0x80000U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_AES_WPOLY, 0x011b, // 0x011b
  aes_sbox_const, /** uint8_t[256] SBOX const packed into uint[64]*/

  // ARRAY types
  (4 << 16) | OP_TYPE_ARRAY, TYPE_ARRAY_UINT_50, TYPE_UINT, CONST_UINT_50, //type: uint4[4]
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
  (4 << 16) | OP_TYPE_STRUCT, TYPE_STRUCT_MUL_ULONG, TYPE_ULONG, TYPE_ULONG,

  // POINTER TYPES
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_WG_UINT, SC_WORKGROUP, TYPE_UINT,   //type: [Workgroup] uint*
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
  (5 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_UINT_UINT_UINT, TYPE_UINT, TYPE_UINT, TYPE_UINT,
  (8 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_UINT4_UINT4_UINT_UINT_UINT_UINT,
              TYPE_UINT4, TYPE_PTR_FN_UINT4, TYPE_UINT, TYPE_UINT, TYPE_UINT, TYPE_UINT, //type: uint4 fn(uint4*,uint,uint,uint,uint)


  // GLOBAL VARIABLES
  (4 << 16) | OP_VARIABLE, TYPE_PTR_IN_UINT3, WORKGROUP_ID, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_IN_UINT3, LOCAL_INVOCATION_ID, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_BF_STATE_BUFFER, PTR_STATE_BUFFER, SC_BUFFER,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_BF_SCRATCHPAD_BUFFER, PTR_SCRATCHPAD_BUFFER, SC_BUFFER,

  // Local(shared) AES tables
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_0, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_1, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_2, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_3, SC_WORKGROUP,

  // FUNCTION IMPORT: AES_ENCODE
  aes_encode_fn,
  // FUNCTION: TO_SCRATCHPAD_IDX
  (5 << 16) | OP_FUNCTION, TYPE_UINT, FUNC_TO_SCRATCHPAD_IDX, FNC_INLINE, TYPE_FUNC_UINT_UINT_UINT,
  (3 << 16) | OP_FUNCTION_PARAMETER, TYPE_UINT, TO_SCRATCHPAD_IDX__ARG_LO,
  (3 << 16) | OP_FUNCTION_PARAMETER, TYPE_UINT, TO_SCRATCHPAD_IDX__ARG_HI,
  (2 << 16) | OP_LABEL, TO_SCRATCHPAD_IDX__LABEL,
  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_UINT, TO_SCRATCHPAD_IDX__X,  TO_SCRATCHPAD_IDX__ARG_LO, CONST_UINT_4,
  (5 << 16) | OP_BITWISE_AND, TYPE_UINT, TO_SCRATCHPAD_IDX__X_AND_0xFFF, TO_SCRATCHPAD_IDX__X, CONST_UINT_0xFFF,
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, TO_SCRATCHPAD_IDX__RESULT, TO_SCRATCHPAD_IDX__X_AND_0xFFF,
              TO_SCRATCHPAD_IDX__ARG_HI, CONST_UINT_12, CONST_UINT_5,
  (2 << 16) | OP_RETURN_VALUE, TO_SCRATCHPAD_IDX__RESULT,
  (1 << 16) | OP_FUNCTION_END,



  // MAIN
  (5 << 16) | OP_FUNCTION, TYPE_VOID, FUNC_MAIN, FNC_NONE, TYPE_FUNC_VOID,
  (2 << 16) | OP_LABEL, LABEL_MAIN,
  // SBOX const array ptr
  (5 << 16) | OP_VARIABLE, TYPE_PTR_FN_CONST_ARRAY_UINT_64, PTR_CONST_AES_SBOX0, SC_FUNCTION, CONST_AES_SBOX0,
  // variable to pass uint4 arg to aes_encode
  (4 << 16) | OP_VARIABLE, TYPE_PTR_FN_UINT4, VAR_AES_ENCODE_IN, SC_FUNCTION,
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

  // calculate aes0,aes1,aes2,aes3 tables
  aes_gen_tables,
  // Bytes 0..31 and 32..63 of the Keccak state
  // are XORed, and the resulting 32 bytes are used to initialize
  // variables a and b, 16 bytes each.
#define load_hash_state(n)                                \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_HASH_STATE_##n, PTR_HASH_STATE, CONST_UINT_##n, \
  (4 << 16) | OP_LOAD, TYPE_UINT, HASH_STATE_##n, PTR_HASH_STATE_##n

#define load_hash_state_vec(n,s0,s1,s2,s3)                                \
  load_hash_state(s0),\
  load_hash_state(s1),  \
  load_hash_state(s2),  \
  load_hash_state(s3),  \
  (7 << 16) | OP_COMPOSITE_CONSTRUCT, TYPE_UINT4, HASH_STATE_V##n, HASH_STATE_##s0, HASH_STATE_##s1, HASH_STATE_##s2, HASH_STATE_##s3

  load_hash_state_vec(0,0,1,2,3),
  load_hash_state_vec(2,8,9,10,11),
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT4, VAL_A0, HASH_STATE_V0, HASH_STATE_V2,

  load_hash_state_vec(1,4,5,6,7),
  load_hash_state_vec(3,12,13,14,15),
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT4, VAL_B0, HASH_STATE_V1, HASH_STATE_V3,

  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT4, PTR_SCRATCHPAD_0, PTR_SCRATCHPAD, CONST_UINT_0,
  (3 << 16) | OP_STORE, PTR_SCRATCHPAD_0, VAL_A0,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT4, PTR_SCRATCHPAD_1, PTR_SCRATCHPAD, CONST_UINT_1,
  (3 << 16) | OP_STORE, PTR_SCRATCHPAD_1, VAL_B0,

  // get scratchpad index from variable uint4 A or B
#define to_scratchpad_idx(a) \
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, XXX_LO_##a, a, 0,\
  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_UINT, XXX1, XXX_LO_##a, CONST_UINT_4,\
  (5 << 16) | OP_BITWISE_AND, TYPE_UINT, XXX1_AND_0xFFF, XXX1, CONST_UINT_0xFFF, \
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, XXX_HI_##a, a, 1,\
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, XXX, XXX1_AND_0xFFF,  XXX_HI_##a, CONST_UINT_12, CONST_UINT_5

  (2 << 16) | OP_BRANCH, LABEL_LOOP_MAIN_START,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN_START,
  //  for (uint i = 0; i < 0x80000 ; ++i) {
  (2 << 16) | OP_BRANCH, LABEL_LOOP_MAIN,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN,
  (7 << 16) | OP_PHI, TYPE_UINT, VAL_LOOP_MAIN_I, CONST_UINT_0, LABEL_LOOP_MAIN_START,
              VAL_LOOP_MAIN_I_INC, LABEL_LOOP_MAIN_BODY,

  (7 << 16) | OP_PHI, TYPE_UINT4, VAL_A, VAL_A0, LABEL_LOOP_MAIN_START, VAL_A0, LABEL_LOOP_MAIN_BODY,
  (7 << 16) | OP_PHI, TYPE_UINT4, VAL_B, VAL_B0, LABEL_LOOP_MAIN_START, VAL_B0, LABEL_LOOP_MAIN_BODY,

  (5 << 16) | OP_ULESS_THAN, TYPE_BOOL, VAL_LOOP_MAIN_COND,  VAL_LOOP_MAIN_I, CONST_UINT_0x80000, // i < 0x80000 ?
  (4 << 16) | OP_LOOP_MERGE, LABEL_LOOP_MAIN_END, LABEL_LOOP_MAIN_BODY, LC_NONE,
  (4 << 16) | OP_BRANCH_CONDITIONAL, VAL_LOOP_MAIN_COND, LABEL_LOOP_MAIN_BODY, LABEL_LOOP_MAIN_END,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN_BODY,
  // uint idx_a = to_scratchpad_idx(a),
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_A_0, VAL_A, 0,
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_A_1, VAL_A, 1,
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_A_2, VAL_A, 2,
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_A_3, VAL_A, 3,
  (6 << 16) | OP_FUNCTION_CALL, TYPE_UINT, VAL_IDX_A, FUNC_TO_SCRATCHPAD_IDX, VAL_A_0, VAL_A_1,

  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT4, PTR_SCRATCHPAD_IDX_A, PTR_SCRATCHPAD, VAL_IDX_A,
  (4 << 16) | OP_LOAD, TYPE_UINT4, VAL_SCRATCHPAD_IDX_A, PTR_SCRATCHPAD_IDX_A,
  (3 << 16) | OP_STORE, VAR_AES_ENCODE_IN, VAL_SCRATCHPAD_IDX_A,
  // uint4 b' = aes_encode(vload4(idx_a, scratchpad), a.s0, a,s1, a,s2, a.s3);
  (9 << 16) | OP_FUNCTION_CALL, TYPE_UINT4, VAL_B_UPD, FUNC_AES_ENCODE, VAR_AES_ENCODE_IN, VAL_A_0, VAL_A_1, VAL_A_2, VAL_A_3,
  // vstore4(b' ^ b, idx_a, scratchpad);
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT4, VAL_B_UPD_XOR_B, VAL_B_UPD, VAL_B,
  (3 << 16) | OP_STORE, PTR_SCRATCHPAD_IDX_A, VAL_B_UPD_XOR_B,
  //  uint idx_b = to_scratchpad_idx(b'),
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_B_UPD_0, VAL_B_UPD, 0,
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_B_UPD_1, VAL_B_UPD, 1,
  (6 << 16) | OP_FUNCTION_CALL, TYPE_UINT, VAL_IDX_B, FUNC_TO_SCRATCHPAD_IDX, VAL_B_UPD_0, VAL_B_UPD_1,

  // ulong ax = as_ulong2(vload4(idx_b, scratchpad)).s0;
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT4, PTR_SCRATCHPAD_IDX_B, PTR_SCRATCHPAD, VAL_IDX_B,
  (4 << 16) | OP_LOAD, TYPE_UINT4, VAL_SCRATCHPAD_IDX_B, PTR_SCRATCHPAD_IDX_B,
  (4 << 16) | OP_BITCAST, TYPE_ULONG2, VAL_SCRATCHPAD_IDX_B_AS_ULONG2, VAL_SCRATCHPAD_IDX_B,
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_ULONG, VAL_AX, VAL_SCRATCHPAD_IDX_B_AS_ULONG2, 0,
  // ulong bx = as_ulong2(b')
  (4 << 16) | OP_BITCAST, TYPE_ULONG2, VAL_B_UPD_AS_ULONG2, VAL_B_UPD,
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_ULONG, VAL_BX, VAL_B_UPD_AS_ULONG2, 0,
  // val cx = ax * bx
/*  (5 << 16) | OP_UMUL_EXTENDED, TYPE_STRUCT_MUL_ULONG, VAL_CX, VAL_AX, VAL_BX,
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_ULONG, VAL_CX_0, VAL_CX, 0,
  (5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_ULONG, VAL_CX_1, VAL_CX, 1,
  // swap two 8-byte halves
  (5 << 16) | OP_COMPOSITE_CONSTRUCT, TYPE_ULONG2, VAL_CX_SWAPPED, VAL_CX_1, VAL_CX_0,
  // add it to `a`
  (4 << 16) | OP_BITCAST, TYPE_ULONG2, VAL_A_AS_ULONG2, VAL_A,
  (5 << 16) | OP_IADD, TYPE_ULONG2, VAL_DX_ULONG2, VAL_CX_SWAPPED, VAL_A_AS_ULONG2,

  // FIXME: below instruction crashes AMD vulkan driver
  (4 << 16) | OP_BITCAST, TYPE_UINT4, VAL_DX, VAL_DX_ULONG2,
  // FIXME: once driver bug is fixed remove code below
  //(4 << 16) | OP_UCONVERT, TYPE_UINT2, VAL_DX_A, VAL_DX_ULONG2,
  //(5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_ULONG2, VAL_DX_ULONG2_SR16, VAL_DX_ULONG2, CONST_UINT_16,
  //(4 << 16) | OP_UCONVERT, TYPE_UINT2, VAL_DX_B, VAL_DX_ULONG2_SR16,
  //(5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_DX_0, VAL_DX_A, 0,
  //(5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_DX_2, VAL_DX_A, 1,
  //(5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_DX_1, VAL_DX_B, 0,
  //(5 << 16) | OP_COMPOSITE_EXTRACT, TYPE_UINT, VAL_DX_3, VAL_DX_B, 1,
  //(7 << 16) | OP_COMPOSITE_CONSTRUCT, TYPE_UINT4, VAL_DX, VAL_DX_0, VAL_DX_1, VAL_DX_2, VAL_DX_3,

  // FIXME: remove code up to here

  // vstore4(X.a, idx_b, scratchpad);
  (3 << 16) | OP_STORE, PTR_SCRATCHPAD_IDX_B, VAL_DX,
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT4, VAL_A_UPD, VAL_DX, VAL_SCRATCHPAD_IDX_B,
*/
  (5 << 16) | OP_IADD, TYPE_UINT, VAL_LOOP_MAIN_I_INC, VAL_LOOP_MAIN_I, CONST_UINT_1,
  (2 << 16) | OP_BRANCH, LABEL_LOOP_MAIN,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN_END,

  // main:return
  (1 << 16) | OP_RETURN,
  (1 << 16) | OP_FUNCTION_END,

};

// clang-format on

const size_t cryptonight_memloop_shader_size =
    sizeof(cryptonight_memloop_shader);
