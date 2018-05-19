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
  WORKGROUP_SIZE,
  // types
  TYPE_VOID,
  TYPE_FUNC_VOID,
  TYPE_FUNC_UINT_UINT_UINT,
  TYPE_FUNC_UINT_UINT,
  TYPE_FUNC_UINT4_UINT4,
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
  TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT_50,
  TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT4_131072,
  TYPE_PTR_BF_STATE_BUFFER,
  TYPE_PTR_BF_ARRAY_UINT_50,
  TYPE_PTR_BF_SCRATCHPAD_BUFFER,
  TYPE_PTR_BF_ARRAY_UINT4_131072,

  // pointers
  PTR_WORKGROUP_X = 50,
  PTR_LOCAL_INVOCATION_X,
  PTR_LOCAL_SIZE,
  PTR_STATE_BUFFER,
  PTR_STATE_BUFFER_INV,
  PTR_SCRATCHPAD_BUFFER,
  PTR_SCRATCHPAD_BUFFER_INV,
  PTR_HASH_STATE,
  PTR_SCRATCHPAD,
  // rotl: local variables
  FUNC_ROTL,
  LABEL_ROTL,
  ROTL_ARG,
  ROTL_NUM_BITS,
  ROTL_OFFSET,
  ROTL_SL,
  ROTL_SR,
  ROTL_RESULT,
  // aes_sub_word
  FUNC_AES_SUB_WORD,
  AES_SUB_WORD_ARG,
  LABEL_AES_SUB_WORD,
#define aes_sub_word_vars(o)                                                   \
  AES_SUB_WORD_X_##o, PTR_AES_SUB_WORD_Y_##o, AES_SUB_WORD_Y_##o,              \
  AES_SUB_WORD_Y_SL_##o, AES_SUB_WORD_Y_BYTE0_##o

  aes_sub_word_vars(0),
  aes_sub_word_vars(8),
  aes_sub_word_vars(16),
  aes_sub_word_vars(24),

  AES_SUB_WORD_Y_OR_0_8,
  AES_SUB_WORD_Y_OR_0_8_16,
  AES_SUB_WORD_Y_OR_0_8_16_24,
  AES_SUB_WORD_RESULT,

  // aes_encode
  FUNC_AES_ENCODE,
  PTR_AES_ENCODE_ARG,
  LABEL_AES_ENCODE,
  LABEL_LOOP_AES_ENCODE,
  LABEL_LOOP_AES_ENCODE_END,
  VAL_AES_ENCODE_LOOP_I,
  VAL_LOOP_AES_ENCODE_COND,
  LABEL_LOOP_AES_ENCODE_BODY,

#define aes_encode_load_k_vars(k)                                              \
  VAL_AES_ENCODE_LOOP_I_PLUS_##k, PTR_VAL_AES_ENCODE_K_##k,                    \
      VAL_AES_ENCODE_RESULT_##k##_K, PTR_VAL_AES_ENCODE_ARG_##k,               \
      VAL_AES_ENCODE_ARG_##k

  aes_encode_load_k_vars(0),
  aes_encode_load_k_vars(1),
  aes_encode_load_k_vars(2),
  aes_encode_load_k_vars(3),
  VAL_AES_ENCODE_LOOP_I_PLUS_4,

#define aes_encode_vars(x, y, k, i)                                            \
  PTR_AES_ENCODE_IDX_##x##_##y, VAL_AES_ENCODE_IDX_##x##_##y,                  \
      PTR_AES_ENCODE_AES_##x##_##y, VAL_AES_ENCODE_AES_##x##_##y,              \
      VAL_AES_ENCODE_RESULT_##i##_##y

  aes_encode_vars(0,0,0,0),
  aes_encode_vars(1,8,1,0),
  aes_encode_vars(2,16,2,0),
  aes_encode_vars(3,24,3,0),

  aes_encode_vars(0,8,0,1),
  aes_encode_vars(1,16,1,1),
  aes_encode_vars(2,24,2,1),
  aes_encode_vars(3,0,3,1),

  aes_encode_vars(0,16,0,2),
  aes_encode_vars(1,24,1,2),
  aes_encode_vars(2,0,2,2),
  aes_encode_vars(3,8,3,2),

  aes_encode_vars(0,24,0,3),
  aes_encode_vars(1,0,1,3),
  aes_encode_vars(2,8,2,3),
  aes_encode_vars(3,16,3,3),

  VAL_AES_ENCODE_ROUND,
  AES_ENCODE_RESULT,
  // main: local variables
  WORKGROUP_X,
  LOCAL_INVOCATION_X,
  LOCAL_INVOCATION_X_SL_8,
  LOCAL_INVOCATION_X_SL_8_PLUS_8,
  LOCAL_SIZE_X,
  PTR_XIN,
  VAL_XIN_0,

#define main_local_vars(n) VAL_BASE_IDX_##n, PTR_HASH_STATE_##n, HASH_STATE_##n
  main_local_vars(0),
  main_local_vars(1),
  main_local_vars(2),
  main_local_vars(3),

  LABEL_LOOP_MAIN,
  LABEL_LOOP_MAIN_BODY,
  LABEL_LOOP_MAIN_END,
  VAL_LOOP_MAIN_COND,
  PTR_SCRATCHPAD_XIN,
  VAL_LOOP_MAIN_I,
  VAL_LOOP_MAIN_INC,

  // constants
  CONST_UINT_0,
  CONST_UINT_1,
  CONST_UINT_2,
  CONST_UINT_3,
  CONST_UINT_4,
  CONST_UINT_5,
  CONST_UINT_7,
  CONST_UINT_8,
  CONST_UINT_10,
  CONST_UINT_12,
  CONST_UINT_16,
  CONST_UINT_24,
  CONST_UINT_32,
  CONST_UINT_40,
  CONST_UINT_50,
  CONST_UINT_64,
  CONST_UINT_0xFF,
  CONST_UINT_256,
  CONST_UINT_131072,
  // AES tables calculation
  CONST_AES_SBOX0,
  CONST_UINT_AES_WPOLY,
  PTR_CONST_AES_SBOX0,
  aes_const_enum, /** from aes_spv.h */
  AES_0,
  AES_1,
  AES_2,
  AES_3,
  AES_KEY,
//  PTR_LOOP_I,
  VAL_LOOP_CALC_AES_CONST_I,
  VAL_LOOP_CALC_AES_CONST_I_INC,
  LABEL_LOOP_CALC_AES_CONST,
  LABEL_LOOP_CALC_AES_CONST_END,
  LABEL_LOOP_CALC_AES_CONST_INC,
  LABEL_LOOP_CALC_AES_CONST_COND,
  LABEL_LOOP_CALC_AES_CONST_BODY,
  VAL_LOOP_CALC_AES_CONST_COND,
  AES_SBOX0_PACKED_GIDX,
  AES_SBOX0_PACKED_LIDX,
  AES_SBOX0_PACKED_OFF,
  PTR_AES_SBOX0_I,
  AES_SBOX0_I,
  AES0_X,
  AES0_X_SL_1,
  AES0_X_MUL_WPOLY,
  AES0_X_BIT7,
  AES0_X_A,
  AES0_X_B,
  AES0_C,
  AES0_C_8_16,
  AES0_C_8_16_24,
  AES1_C,
  AES2_C,
  AES3_C,
  PTR_AES_0_I,
  PTR_AES_1_I,
  PTR_AES_2_I,
  PTR_AES_3_I,
  // aes gen key loop
  PTR_HASH_STATE_LINV,
  HASH_STATE_LINV,
  PTR_AES_KEY_LINV,
  VAL_LOOP_GEN_AES_KEY_I,
  VAL_LOOP_GEN_AES_KEY_I_INC,
  VAL_LOOP_GEN_AES_KEY_I_DEC,
  LABEL_LOOP_GEN_AES_KEY,
  LABEL_LOOP_GEN_AES_KEY_END,
  LABEL_LOOP_GEN_AES_KEY_BODY,
  VAL_LOOP_GEN_AES_KEY_COND,
  PTR_AES_KEY_I,
  AES_KEY_I,
  VAL_AES_KEY_I,
  PTR_AES_KEY_I_SUB_1,
  VAL_AES_KEY_I_SUB_1,
  VAL_AES_KEY_I_SUB_1_SW,
  VAL_LOOP_GEN_AES_KEY_I_AND_3,
  VAL_LOOP_GEN_AES_KEY_I_AND_3_NEQ_0,
  VAL_LOOP_GEN_AES_KEY_T,
  VAL_LOOP_GEN_AES_KEY_I_SR_3,
  VAL_LOOP_GEN_AES_KEY_I_SR_3_SUB_1,
  VAL_LOOP_GEN_AES_KEY_RC,
  VAL_LOOP_GEN_AES_KEY_I_ROTL_24,
  VAL_LOOP_GEN_AES_KEY_S,
  VAL_LOOP_GEN_AES_KEY_I_AND_7,
  VAL_LOOP_GEN_AES_KEY_I_AND_7_NEQ_0,
  VAL_LOOP_GEN_AES_KEY_Z,
  VAL_LOOP_GEN_AES_KEY_I_SUB_8,
  PTR_AES_KEY_I_SUB_8,
  AES_KEY_I_SUB_8,

  LOCAL_INVOCATION_X_PLUS_4,
  TYPE_PTR_BF_UINT4,
  VAL_AES_RES,
  BOUND
};

// clang-format off
const uint32_t cryptonight_explode_shader[] = {
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
  (4 << 16) | OP_DECORATE, WORKGROUP_SIZE, DECOR_BUILTIN, BUILTIN_WORKGROUP_SIZE,
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
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_5, 5, // 5U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_7, 7, // 7U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_8, 8, // 8U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_10, 10, // 10U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_12, 12, // 12U
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
  (67 << 16)| OP_CONSTANT_COMPOSITE, TYPE_CONST_ARRAY_UINT_64, CONST_AES_SBOX0, aes_const_enum,

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
  (4 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_UINT_UINT, TYPE_UINT, TYPE_UINT,  //type: uint fn(uint)
  (5 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_UINT_UINT_UINT, TYPE_UINT, TYPE_UINT, TYPE_UINT,  //type: uint fn(uint,uint)
  (4 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_UINT4_UINT4, TYPE_UINT4, TYPE_PTR_FN_UINT4,  //type: uint4 fn(uint4*)

  // GLOBAL VARIABLES
  (4 << 16) | OP_VARIABLE, TYPE_PTR_IN_UINT3, WORKGROUP_ID, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_IN_UINT3, LOCAL_INVOCATION_ID, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_IN_UINT3, WORKGROUP_SIZE, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_BF_STATE_BUFFER, PTR_STATE_BUFFER, SC_BUFFER,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_BF_SCRATCHPAD_BUFFER, PTR_SCRATCHPAD_BUFFER, SC_BUFFER,

  // Local(shared) AES tables and 10 rounds key
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_0, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_1, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_2, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_256, AES_3, SC_WORKGROUP,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_WG_ARRAY_UINT_40, AES_KEY, SC_WORKGROUP, /** 10 rounds key */

  // uint rotate_left_32(uint arg, uint num_bits)
  (5 << 16) | OP_FUNCTION, TYPE_UINT, FUNC_ROTL, FNC_INLINE, TYPE_FUNC_UINT_UINT_UINT,
  (3 << 16) | OP_FUNCTION_PARAMETER, TYPE_UINT, ROTL_ARG,
  (3 << 16) | OP_FUNCTION_PARAMETER, TYPE_UINT, ROTL_NUM_BITS,
  (2 << 16) | OP_LABEL, LABEL_ROTL,
  (5 << 16) | OP_ISUB, TYPE_UINT, ROTL_OFFSET, CONST_UINT_32, ROTL_NUM_BITS,
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, ROTL_SL, ROTL_ARG, ROTL_NUM_BITS,
  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_UINT, ROTL_SR, ROTL_ARG, ROTL_OFFSET,
  (5 << 16) | OP_BITWISE_OR, TYPE_UINT, ROTL_RESULT, ROTL_SL, ROTL_SR,
  (2 << 16) | OP_RETURN_VALUE, ROTL_RESULT,
  (1 << 16) | OP_FUNCTION_END,

  // FUNCTION :uint aes_sub_word(uint key)
  //    return (AES_SBOX[BYTE3(key)] << 24) | (AES_SBOX[BYTE2(key)] << 16) | (AES_SBOX[BYTE1(key)] << 8) | AES_SBOX[BYTE0(key)];
  (5 << 16) | OP_FUNCTION, TYPE_UINT, FUNC_AES_SUB_WORD, FNC_INLINE, TYPE_FUNC_UINT_UINT,
  (3 << 16) | OP_FUNCTION_PARAMETER, TYPE_UINT, AES_SUB_WORD_ARG,
  (2 << 16) | OP_LABEL, LABEL_AES_SUB_WORD,

#define aes_sbox_get_byte(o) \
  (6 << 16) | OP_BITFIELD_UEXTRACT, TYPE_UINT, AES_SUB_WORD_X_##o, AES_SUB_WORD_ARG, CONST_UINT_##o, CONST_UINT_8, \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_AES_SUB_WORD_Y_##o, AES_2, AES_SUB_WORD_X_##o, \
  (4 << 16) | OP_LOAD, TYPE_UINT, AES_SUB_WORD_Y_##o, PTR_AES_SUB_WORD_Y_##o, \
  (5 << 16) | OP_BITWISE_AND, TYPE_UINT, AES_SUB_WORD_Y_BYTE0_##o, AES_SUB_WORD_Y_##o, CONST_UINT_0xFF, \
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, AES_SUB_WORD_Y_SL_##o, AES_SUB_WORD_Y_BYTE0_##o, CONST_UINT_##o

  aes_sbox_get_byte(0),
  aes_sbox_get_byte(8),
  aes_sbox_get_byte(16),
  aes_sbox_get_byte(24),

  (5 << 16) | OP_BITWISE_OR, TYPE_UINT, AES_SUB_WORD_Y_OR_0_8, AES_SUB_WORD_Y_SL_0, AES_SUB_WORD_Y_SL_8,
  (5 << 16) | OP_BITWISE_OR, TYPE_UINT, AES_SUB_WORD_Y_OR_0_8_16, AES_SUB_WORD_Y_OR_0_8, AES_SUB_WORD_Y_SL_16,
  (5 << 16) | OP_BITWISE_OR, TYPE_UINT, AES_SUB_WORD_RESULT, AES_SUB_WORD_Y_OR_0_8_16, AES_SUB_WORD_Y_SL_24,

  (2 << 16) | OP_RETURN_VALUE, AES_SUB_WORD_RESULT,
  (1 << 16) | OP_FUNCTION_END,

  // perform 10 aes rounds with AES_KEY, using AES_0, AES_1, AES_2, AES_3 tables
  // FUNCTION : uint4 aes_encode(uint4)
  (5 << 16) | OP_FUNCTION, TYPE_UINT4, FUNC_AES_ENCODE, FNC_INLINE, TYPE_FUNC_UINT4_UINT4,
  (3 << 16) | OP_FUNCTION_PARAMETER, TYPE_PTR_FN_UINT4, PTR_AES_ENCODE_ARG,

  (2 << 16) | OP_LABEL, LABEL_AES_ENCODE,
  (2 << 16) | OP_BRANCH, LABEL_LOOP_AES_ENCODE,
  (2 << 16) | OP_LABEL, LABEL_LOOP_AES_ENCODE,
  (7 << 16) | OP_PHI, TYPE_UINT, VAL_AES_ENCODE_LOOP_I, CONST_UINT_0, LABEL_AES_ENCODE,
              VAL_AES_ENCODE_LOOP_I_PLUS_4, LABEL_LOOP_AES_ENCODE_BODY,

  (5 << 16) | OP_ULESS_THAN, TYPE_BOOL, VAL_LOOP_AES_ENCODE_COND, VAL_AES_ENCODE_LOOP_I, CONST_UINT_40, // i < 40 ?
  (4 << 16) | OP_LOOP_MERGE, LABEL_LOOP_AES_ENCODE_END, LABEL_LOOP_AES_ENCODE_BODY, LC_UNROLL,
  (4 << 16) | OP_BRANCH_CONDITIONAL, VAL_LOOP_AES_ENCODE_COND, LABEL_LOOP_AES_ENCODE_BODY, LABEL_LOOP_AES_ENCODE_END,
  (2 << 16) | OP_LABEL, LABEL_LOOP_AES_ENCODE_BODY,

#define aes_encode_load_k(k)                                            \
  (5 << 16) | OP_IADD, TYPE_UINT, VAL_AES_ENCODE_LOOP_I_PLUS_##k, VAL_AES_ENCODE_LOOP_I, CONST_UINT_##k,            \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_VAL_AES_ENCODE_K_##k, AES_KEY, VAL_AES_ENCODE_LOOP_I_PLUS_##k, \
  (4 << 16) | OP_LOAD, TYPE_UINT, VAL_AES_ENCODE_RESULT_##k##_K, PTR_VAL_AES_ENCODE_K_##k,\
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_FN_UINT, PTR_VAL_AES_ENCODE_ARG_##k, PTR_AES_ENCODE_ARG, CONST_UINT_##k, \
  (4 << 16) | OP_LOAD, TYPE_UINT, VAL_AES_ENCODE_ARG_##k, PTR_VAL_AES_ENCODE_ARG_##k

  aes_encode_load_k(0),
  aes_encode_load_k(1),
  aes_encode_load_k(2),
  aes_encode_load_k(3),

#define aes_encode_lookup_table(a,x,o,k,i)                              \
  (6 << 16) | OP_BITFIELD_UEXTRACT, TYPE_UINT, VAL_AES_ENCODE_IDX_##x##_##o, VAL_AES_ENCODE_ARG_##x, CONST_UINT_##o, CONST_UINT_8, \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_AES_ENCODE_AES_##x##_##o, AES_##a, VAL_AES_ENCODE_IDX_##x##_##o, \
  (4 << 16) | OP_LOAD, TYPE_UINT, VAL_AES_ENCODE_AES_##x##_##o, PTR_AES_ENCODE_AES_##x##_##o, \
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT, VAL_AES_ENCODE_RESULT_##i##_##o, VAL_AES_ENCODE_RESULT_##i##_##k, VAL_AES_ENCODE_AES_##x##_##o

  // AES0[BYTE0(i0)] ^ AES1[BYTE1(i1)] ^ AES2[BYTE2(i2)] ^ AES3[BYTE3(i3)] ^ k[0]
  aes_encode_lookup_table(0,0,0,K,0),
  aes_encode_lookup_table(1,1,8,0,0),
  aes_encode_lookup_table(2,2,16,8,0),
  aes_encode_lookup_table(3,3,24,16,0),

  // AES0[BYTE0(i1)] ^ AES1[BYTE1(i2)] ^ AES2[BYTE2(i3)] ^ AES3[BYTE3(i0)] ^ k[1]
  aes_encode_lookup_table(0,1,0,K,1),
  aes_encode_lookup_table(1,2,8,0,1),
  aes_encode_lookup_table(2,3,16,8,1),
  aes_encode_lookup_table(3,0,24,16,1),

  // AES0[BYTE0(i2)] ^ AES1[BYTE1(i3)] ^ AES2[BYTE2(i0)] ^ AES3[BYTE3(i1)] ^ k[2]
  aes_encode_lookup_table(0,2,0,K,2),
  aes_encode_lookup_table(1,3,8,0,2),
  aes_encode_lookup_table(2,0,16,8,2),
  aes_encode_lookup_table(3,1,24,16,2),

  // AES0[BYTE0(i3)] ^ AES1[BYTE1(i0)] ^ AES2[BYTE2(i1)] ^ AES3[BYTE3(i2)] ^ k[3]
  aes_encode_lookup_table(0,3,0,K,3),
  aes_encode_lookup_table(1,0,8,0,3),
  aes_encode_lookup_table(2,1,16,8,3),
  aes_encode_lookup_table(3,2,24,16,3),

  (7 << 16) | OP_COMPOSITE_CONSTRUCT, TYPE_UINT4, VAL_AES_ENCODE_ROUND, VAL_AES_ENCODE_RESULT_0_24, VAL_AES_ENCODE_RESULT_1_24,
              VAL_AES_ENCODE_RESULT_2_24, VAL_AES_ENCODE_RESULT_3_24,

  (3 << 16) | OP_STORE, PTR_AES_ENCODE_ARG, VAL_AES_ENCODE_ROUND,

  (5 << 16) | OP_IADD, TYPE_UINT, VAL_AES_ENCODE_LOOP_I_PLUS_4, VAL_AES_ENCODE_LOOP_I, CONST_UINT_4,
  (2 << 16) | OP_BRANCH, LABEL_LOOP_AES_ENCODE,
  (2 << 16) | OP_LABEL, LABEL_LOOP_AES_ENCODE_END,

  (4 << 16) | OP_LOAD, TYPE_UINT4, AES_ENCODE_RESULT, PTR_AES_ENCODE_ARG,
  (2 << 16) | OP_RETURN_VALUE, AES_ENCODE_RESULT,
  (1 << 16) | OP_FUNCTION_END,


  // MAIN
  (5 << 16) | OP_FUNCTION, TYPE_VOID, FUNC_MAIN, FNC_NONE, TYPE_FUNC_VOID,
  (2 << 16) | OP_LABEL, LABEL_MAIN,
  // variables
  (4 << 16) | OP_VARIABLE, TYPE_PTR_FN_UINT4, PTR_XIN, SC_FUNCTION,
  (5 << 16) | OP_VARIABLE, TYPE_PTR_FN_CONST_ARRAY_UINT_64, PTR_CONST_AES_SBOX0, SC_FUNCTION, CONST_AES_SBOX0,
  // get global invocation index
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_IN_UINT, PTR_WORKGROUP_X, WORKGROUP_ID, CONST_UINT_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, WORKGROUP_X, PTR_WORKGROUP_X,
  // get local invocation index
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_IN_UINT, PTR_LOCAL_INVOCATION_X, LOCAL_INVOCATION_ID, CONST_UINT_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, LOCAL_INVOCATION_X, PTR_LOCAL_INVOCATION_X,
  // get local size
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_IN_UINT, PTR_LOCAL_SIZE, WORKGROUP_SIZE, CONST_UINT_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, LOCAL_SIZE_X, PTR_LOCAL_SIZE,

  // get pointer to HASH_STATE array for current invocation
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT_50, PTR_STATE_BUFFER_INV, PTR_STATE_BUFFER, CONST_UINT_0,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_ARRAY_UINT_50, PTR_HASH_STATE, PTR_STATE_BUFFER_INV, WORKGROUP_X,
  // get pointer to SCRATCHPAD array for current invocation
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT4_131072, PTR_SCRATCHPAD_BUFFER_INV, PTR_SCRATCHPAD_BUFFER, CONST_UINT_0,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_ARRAY_UINT4_131072, PTR_SCRATCHPAD, PTR_SCRATCHPAD_BUFFER_INV, WORKGROUP_X,
  // copy first 32 bytes of state to aes_key array
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_HASH_STATE_LINV, PTR_HASH_STATE, LOCAL_INVOCATION_X,
  (4 << 16) | OP_LOAD, TYPE_UINT, HASH_STATE_LINV, PTR_HASH_STATE_LINV,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_AES_KEY_LINV, AES_KEY, LOCAL_INVOCATION_X,
  (3 << 16) | OP_STORE, PTR_AES_KEY_LINV, HASH_STATE_LINV,

  // calculate aes0,aes1,aes2,aes3 tables
  // for (size_t i = get_local_id(0); i < 256; i += get_local_size(0))

  (2 << 16) | OP_BRANCH, LABEL_LOOP_CALC_AES_CONST,
  (2 << 16) | OP_LABEL, LABEL_LOOP_CALC_AES_CONST,
  (7 << 16) | OP_PHI, TYPE_UINT, VAL_LOOP_CALC_AES_CONST_I, LOCAL_INVOCATION_X, LABEL_MAIN,
              VAL_LOOP_CALC_AES_CONST_I_INC, LABEL_LOOP_CALC_AES_CONST_BODY,
  (5 << 16) | OP_ULESS_THAN, TYPE_BOOL, VAL_LOOP_CALC_AES_CONST_COND,  VAL_LOOP_CALC_AES_CONST_I, CONST_UINT_256, // i < 256 ?
  (4 << 16) | OP_LOOP_MERGE, LABEL_LOOP_CALC_AES_CONST_END, LABEL_LOOP_CALC_AES_CONST_BODY, LC_UNROLL,
  (4 << 16) | OP_BRANCH_CONDITIONAL, VAL_LOOP_CALC_AES_CONST_COND, LABEL_LOOP_CALC_AES_CONST_BODY, LABEL_LOOP_CALC_AES_CONST_END,
  (2 << 16) | OP_LABEL, LABEL_LOOP_CALC_AES_CONST_BODY,

  // unpack AES_0 const
  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_UINT, AES_SBOX0_PACKED_GIDX, VAL_LOOP_CALC_AES_CONST_I, CONST_UINT_2,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_FN_UINT, PTR_AES_SBOX0_I, PTR_CONST_AES_SBOX0, AES_SBOX0_PACKED_GIDX,
  (4 << 16) | OP_LOAD, TYPE_UINT, AES_SBOX0_I, PTR_AES_SBOX0_I,
  (5 << 16) | OP_BITWISE_AND, TYPE_UINT, AES_SBOX0_PACKED_LIDX, VAL_LOOP_CALC_AES_CONST_I, CONST_UINT_3,
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, AES_SBOX0_PACKED_OFF, AES_SBOX0_PACKED_LIDX, CONST_UINT_3,
  (6 << 16) | OP_BITFIELD_UEXTRACT, TYPE_UINT, AES0_X, AES_SBOX0_I, AES_SBOX0_PACKED_OFF, CONST_UINT_8,
  // x = AES_SBOX0[i]
  // a = (x << 1) ^ (bitextract(x, 8, 1) * AES_WPOLY)
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, AES0_X_SL_1, AES0_X, CONST_UINT_1,
  (6 << 16) | OP_BITFIELD_UEXTRACT, TYPE_UINT, AES0_X_BIT7, AES0_X, CONST_UINT_7, CONST_UINT_1,
  (5 << 16) | OP_IMUL, TYPE_UINT, AES0_X_MUL_WPOLY, AES0_X_BIT7, CONST_UINT_AES_WPOLY,
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT, AES0_X_A, AES0_X_SL_1, AES0_X_MUL_WPOLY,
  // b = a ^ x
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT, AES0_X_B, AES0_X_A, AES0_X,
  // AES_0[i] = uint(a,x,x,b)
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, AES0_C_8_16, AES0_X_A, AES0_X, CONST_UINT_8, CONST_UINT_8,
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, AES0_C_8_16_24, AES0_C_8_16, AES0_X, CONST_UINT_16, CONST_UINT_8,
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, AES0_C, AES0_C_8_16_24, AES0_X_B, CONST_UINT_24, CONST_UINT_8,

  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_AES_0_I, AES_0, VAL_LOOP_CALC_AES_CONST_I,
  (3 << 16) | OP_STORE, PTR_AES_0_I, AES0_C,
#define aes_const_rotate(c,n) \
  (6 << 16) | OP_FUNCTION_CALL, TYPE_UINT, AES##c##_C, FUNC_ROTL, AES0_C, CONST_UINT_##n, \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_AES_##c##_I, AES_##c, VAL_LOOP_CALC_AES_CONST_I, \
  (3 << 16) | OP_STORE, PTR_AES_##c##_I, AES##c##_C

  aes_const_rotate(1,8),
  aes_const_rotate(2,16),
  aes_const_rotate(3,24),

  (5 << 16) | OP_IADD, TYPE_UINT, VAL_LOOP_CALC_AES_CONST_I_INC, VAL_LOOP_CALC_AES_CONST_I, LOCAL_SIZE_X, // ; i += local_size
  (2 << 16) | OP_BRANCH, LABEL_LOOP_CALC_AES_CONST,
  (2 << 16) | OP_LABEL, LABEL_LOOP_CALC_AES_CONST_END,
  // end aes const calculation loop

  (4 << 16) | OP_CONTROL_BARRIER, CONST_UINT_2, CONST_UINT_2, CONST_UINT_256,
  // expand 32 bit aes key to 10 round keys
  // this is not parallelizable (only 1 local thread with id=0 will do the work)
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, LOCAL_INVOCATION_X_SL_8, LOCAL_INVOCATION_X, CONST_UINT_8,
  (5 << 16) | OP_IADD, TYPE_UINT, LOCAL_INVOCATION_X_SL_8_PLUS_8, LOCAL_INVOCATION_X_SL_8, CONST_UINT_8,
  // for (i = 8; i < 40; ++i)
  (2 << 16) | OP_BRANCH, LABEL_LOOP_GEN_AES_KEY,
  (2 << 16) | OP_LABEL, LABEL_LOOP_GEN_AES_KEY,
  (7 << 16) | OP_PHI, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_I, LOCAL_INVOCATION_X_SL_8_PLUS_8, LABEL_LOOP_CALC_AES_CONST_END,
              VAL_LOOP_GEN_AES_KEY_I_INC, LABEL_LOOP_GEN_AES_KEY_BODY,
  (5 << 16) | OP_ULESS_THAN, TYPE_BOOL, VAL_LOOP_GEN_AES_KEY_COND, VAL_LOOP_GEN_AES_KEY_I, CONST_UINT_40, // i < 40 ?
  (4 << 16) | OP_LOOP_MERGE, LABEL_LOOP_GEN_AES_KEY_END, LABEL_LOOP_GEN_AES_KEY_BODY, LC_UNROLL,
  (4 << 16) | OP_BRANCH_CONDITIONAL, VAL_LOOP_GEN_AES_KEY_COND, LABEL_LOOP_GEN_AES_KEY_BODY, LABEL_LOOP_GEN_AES_KEY_END,
  (2 << 16) | OP_LABEL, LABEL_LOOP_GEN_AES_KEY_BODY,
  // {
  // key[i]
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_AES_KEY_I, AES_KEY, VAL_LOOP_GEN_AES_KEY_I,
  // key[i - 1]
  (5 << 16) | OP_ISUB, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_I_DEC, VAL_LOOP_GEN_AES_KEY_I, CONST_UINT_1,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_AES_KEY_I_SUB_1, AES_KEY, VAL_LOOP_GEN_AES_KEY_I_DEC,
  (4 << 16) | OP_LOAD, TYPE_UINT, VAL_AES_KEY_I_SUB_1, PTR_AES_KEY_I_SUB_1,
  // sub_word(key[i - 1])
  (5 << 16) | OP_FUNCTION_CALL, TYPE_UINT, VAL_AES_KEY_I_SUB_1_SW, FUNC_AES_SUB_WORD, VAL_AES_KEY_I_SUB_1,

  // For 256-bit keys, an sbox permutation is done every other 4th uint generated
  // t = i & 3 ? key[i - 1] : sub_word(key[i - 1])
  (5 << 16) | OP_BITWISE_AND, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_I_AND_3, VAL_LOOP_GEN_AES_KEY_I, CONST_UINT_3,
  (5 << 16) | OP_INOTEQUAL, TYPE_BOOL, VAL_LOOP_GEN_AES_KEY_I_AND_3_NEQ_0, VAL_LOOP_GEN_AES_KEY_I_AND_3, CONST_UINT_0,
  (6 << 16) | OP_SELECT, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_T, VAL_LOOP_GEN_AES_KEY_I_AND_3_NEQ_0,
              VAL_AES_KEY_I_SUB_1, VAL_AES_KEY_I_SUB_1_SW,

  // rc = 1 << ((i >> 3) - 1);
  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_I_SR_3, VAL_LOOP_GEN_AES_KEY_I, CONST_UINT_3,
  (5 << 16) | OP_ISUB, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_I_SR_3_SUB_1, VAL_LOOP_GEN_AES_KEY_I_SR_3, CONST_UINT_1,
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_RC, CONST_UINT_1, VAL_LOOP_GEN_AES_KEY_I_SR_3_SUB_1,
  // s = rotl(t, 24U) ^ rc
  (6 << 16) | OP_FUNCTION_CALL, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_I_ROTL_24, FUNC_ROTL, VAL_LOOP_GEN_AES_KEY_T, CONST_UINT_24,
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_S, VAL_LOOP_GEN_AES_KEY_I_ROTL_24, VAL_LOOP_GEN_AES_KEY_RC,
  // z = i & 7 ? t : s
  (5 << 16) | OP_BITWISE_AND, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_I_AND_7, VAL_LOOP_GEN_AES_KEY_I, CONST_UINT_7,
  (5 << 16) | OP_INOTEQUAL, TYPE_BOOL, VAL_LOOP_GEN_AES_KEY_I_AND_7_NEQ_0, VAL_LOOP_GEN_AES_KEY_I_AND_7, CONST_UINT_0,
  (6 << 16) | OP_SELECT, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_Z, VAL_LOOP_GEN_AES_KEY_I_AND_7_NEQ_0,
              VAL_LOOP_GEN_AES_KEY_T, VAL_LOOP_GEN_AES_KEY_S,
  //  k[i] = k[i - 8] ^ z;
  (5 << 16) | OP_ISUB, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_I_SUB_8, VAL_LOOP_GEN_AES_KEY_I, CONST_UINT_8,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_AES_KEY_I_SUB_8, AES_KEY, VAL_LOOP_GEN_AES_KEY_I_SUB_8,
  (4 << 16) | OP_LOAD, TYPE_UINT, AES_KEY_I_SUB_8, PTR_AES_KEY_I_SUB_8,
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT, AES_KEY_I, AES_KEY_I_SUB_8, VAL_LOOP_GEN_AES_KEY_Z,
  (3 << 16) | OP_STORE, PTR_AES_KEY_I, AES_KEY_I,
  // }
  (5 << 16) | OP_IADD, TYPE_UINT, VAL_LOOP_GEN_AES_KEY_I_INC, VAL_LOOP_GEN_AES_KEY_I, CONST_UINT_1, // ; ++i
  (2 << 16) | OP_BRANCH, LABEL_LOOP_GEN_AES_KEY,
  (2 << 16) | OP_LABEL, LABEL_LOOP_GEN_AES_KEY_END,
  // end aes gen key loop
  (4 << 16) | OP_CONTROL_BARRIER, CONST_UINT_2, CONST_UINT_2, CONST_UINT_256,

  // The bytes 64..191 are extracted from the Keccak final state and split into 8 blocks of 16 bytes each.
  // 1 block per local thread
  // base_idx = 16 + local_idx * 4 => 4 * (local_idx + 4)
  (5 << 16) | OP_IADD, TYPE_UINT, LOCAL_INVOCATION_X_PLUS_4, LOCAL_INVOCATION_X, CONST_UINT_4,
  (5 << 16) | OP_IMUL, TYPE_UINT, VAL_BASE_IDX_0, LOCAL_INVOCATION_X_PLUS_4, CONST_UINT_4,

  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_HASH_STATE_0, PTR_HASH_STATE, VAL_BASE_IDX_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, HASH_STATE_0, PTR_HASH_STATE_0,

#define load_state_word(c)                                              \
  (5 << 16) | OP_IADD, TYPE_UINT, VAL_BASE_IDX_##c, VAL_BASE_IDX_0, CONST_UINT_##c, \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_HASH_STATE_##c, PTR_HASH_STATE, VAL_BASE_IDX_##c, \
  (4 << 16) | OP_LOAD, TYPE_UINT, HASH_STATE_##c, PTR_HASH_STATE_##c

  load_state_word(1),
  load_state_word(2),
  load_state_word(3),

  (7 << 16) | OP_COMPOSITE_CONSTRUCT, TYPE_UINT4, VAL_XIN_0, HASH_STATE_0, HASH_STATE_1, HASH_STATE_2, HASH_STATE_3,
  (3 << 16) | OP_STORE, PTR_XIN, VAL_XIN_0,

  //  for (uint i = local_id(0); i < CRYPTONIGHT_MEMORY_UINT; i += 8) {
  (2 << 16) | OP_BRANCH, LABEL_LOOP_MAIN,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN,
  (7 << 16) | OP_PHI, TYPE_UINT, VAL_LOOP_MAIN_I, LOCAL_INVOCATION_X, LABEL_LOOP_GEN_AES_KEY_END,
              VAL_LOOP_MAIN_INC, LABEL_LOOP_MAIN_BODY,

  (5 << 16) | OP_ULESS_THAN, TYPE_BOOL, VAL_LOOP_MAIN_COND,  VAL_LOOP_MAIN_I, CONST_UINT_131072, // i < 131072 ?
  (4 << 16) | OP_LOOP_MERGE, LABEL_LOOP_MAIN_END, LABEL_LOOP_MAIN_BODY, LC_NONE,
  (4 << 16) | OP_BRANCH_CONDITIONAL, VAL_LOOP_MAIN_COND, LABEL_LOOP_MAIN_BODY, LABEL_LOOP_MAIN_END,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN_BODY,

  (5 << 16) | OP_FUNCTION_CALL, TYPE_UINT4, VAL_AES_RES, FUNC_AES_ENCODE, PTR_XIN,
  (3 << 16) | OP_STORE, PTR_XIN, VAL_AES_RES,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT4, PTR_SCRATCHPAD_XIN, PTR_SCRATCHPAD, VAL_LOOP_MAIN_I,
  (3 << 16) | OP_STORE, PTR_SCRATCHPAD_XIN, VAL_AES_RES,

  (5 << 16) | OP_IADD, TYPE_UINT, VAL_LOOP_MAIN_INC, VAL_LOOP_MAIN_I, CONST_UINT_8,
  (2 << 16) | OP_BRANCH, LABEL_LOOP_MAIN,
  (2 << 16) | OP_LABEL, LABEL_LOOP_MAIN_END,
/*

  (5 << 16) | OP_IMUL, TYPE_UINT, IDX0, LOCAL_INVOCATION_X, CONST_UINT_4,
  (5 << 16) | OP_IADD, TYPE_UINT, IDX1, IDX0, CONST_UINT_1,
  (5 << 16) | OP_IADD, TYPE_UINT, IDX2, IDX0, CONST_UINT_2,
  (5 << 16) | OP_IADD, TYPE_UINT, IDX3, IDX0, CONST_UINT_3,


  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_YYY0, AES_KEY, IDX0,
  (4 << 16) | OP_LOAD, TYPE_UINT, YYY0, PTR_YYY0,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_YYY1, AES_KEY, IDX1,
  (4 << 16) | OP_LOAD, TYPE_UINT, YYY1, PTR_YYY1,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_YYY2, AES_KEY, IDX2,
  (4 << 16) | OP_LOAD, TYPE_UINT, YYY2, PTR_YYY2,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_WG_UINT, PTR_YYY3, AES_KEY, IDX3,
  (4 << 16) | OP_LOAD, TYPE_UINT, YYY3, PTR_YYY3,

  (7 << 16) | OP_COMPOSITE_CONSTRUCT, TYPE_UINT4, XXX, YYY0, YYY1, YYY2, YYY3,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT4, PTR_XXX, PTR_SCRATCHPAD, LOCAL_INVOCATION_X,
  (3 << 16) | OP_STORE, PTR_XXX, XXX,
*/

  // main:return
  (1 << 16) | OP_RETURN,
  (1 << 16) | OP_FUNCTION_END,
};

// clang-format on

const size_t cryptonight_explode_shader_size =
    sizeof(cryptonight_explode_shader);
