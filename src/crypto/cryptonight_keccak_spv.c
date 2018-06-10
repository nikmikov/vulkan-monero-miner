#include "utils/spirv.h"

#include "crypto/cryptonight_spv.h"

enum { // variables
  RESERVED_ID = 0,
  EXT_INST_GLSL_STD_450,
  FUNC_MAIN,
  LABEL_MAIN,
  LABEL_THETA_FOLD5_XOR,
  LABEL_LOOP,
  LABEL_LOOP_COND,
  LABEL_LOOP_INC,
  LABEL_LOOP_BODY,
  LABEL_LOOP_END,
  // global variables
  GLOBAL_INVOCATION_ID,
  // types
  TYPE_VOID,
  TYPE_FUNC_VOID,
  TYPE_FUNC_ULONG_ULONG_UINT,
  TYPE_BOOL,
  TYPE_UINT,
  TYPE_ULONG,
  TYPE_UINT3,
  TYPE_PTR_UINT_INPUT,
  TYPE_PTR_UINT_FUNCTION,
  TYPE_PTR_UINT3_INPUT,
  TYPE_ARRAY_ULONG_25,
  TYPE_PTR_ARRAY_ULONG_25,
  TYPE_RT_ARRAY_ARRAY_ULONG_25,
  TYPE_PTR_RT_ARRAY_ARRAY_ULONG_25,
  TYPE_STRUCT_BUFFER,
  TYPE_PTR_BUFFER,
  TYPE_PTR_ULONG_BUFFER,
  TYPE_PTR_ULONG_FUNCTION,

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
  CONST_UINT_17,
  CONST_UINT_18,
  CONST_UINT_19,
  CONST_UINT_20,
  CONST_UINT_21,
  CONST_UINT_22,
  CONST_UINT_23,
  CONST_UINT_24,
  CONST_UINT_25,
  CONST_UINT_27,
  CONST_UINT_28,
  CONST_UINT_31,
  CONST_UINT_36,
  CONST_UINT_39,
  CONST_UINT_41,
  CONST_UINT_43,
  CONST_UINT_44,
  CONST_UINT_45,
  CONST_UINT_55,
  CONST_UINT_56,
  CONST_UINT_61,
  CONST_UINT_62,
  CONST_UINT_63,
  CONST_UINT_64,
  CONST_UINT_0x71,
  CONST_UINT_0x80,
  CONST_ULONG_0,
  CONST_ULONG_1,
  // pointers
  PTR_GLOBAL_INVOCATION_X,
  PTR_BUFFER,
  PTR_BUFFER_INV,
  PTR_HASH_STATE,
  // rotl64: local variables
  FUNC_ROTL64,
  LABEL_ROTL64,
  ROTL64_ARG,
  ROTL64_NUM_BITS,
  ROTL64_OFFSET,
  ROTL64_SL,
  ROTL64_SR,
  ROTL64_RESULT,
  // main: local variables
  GLOBAL_INVOCATION_X,
#define local_vars(x)                                                          \
  PTR_HASH_STATE_i##x, HASH_STATE_i##x, VAR_B##x, VAR_B##x##_0, VAR_B##x##_1,  \
      VAR_B##x##_2, VAR_B##x##_3, VAR_B##x##_RL1, VAR_BC##x, VAR_THETA_##x,    \
      VAR_RHO_##x, VAR_NOT_RHO_##x, VAR_CHI_S_##x, VAR_CHI_##x
  local_vars(0),
  local_vars(1),
  local_vars(2),
  local_vars(3),
  local_vars(4),
  local_vars(5),
  local_vars(6),
  local_vars(7),
  local_vars(8),
  local_vars(9),
  local_vars(10),
  local_vars(11),
  local_vars(12),
  local_vars(13),
  local_vars(14),
  local_vars(15),
  local_vars(16),
  local_vars(17),
  local_vars(18),
  local_vars(19),
  local_vars(20),
  local_vars(21),
  local_vars(22),
  local_vars(23),
  local_vars(24),
  VAR_IOTA_0,

  // keccak rndc calculation
  PTR_LFSR,
  LFSR_S,
#define keccak_vars(n)                                                         \
  LFSR_##n, LFSR_BIT_0_##n, LFSR_BIT_0_SET_##n, LFSR_AND_0x80_##n,             \
      LFSR_BIT_7_SET_##n, LFSR_SL1_##n, LFSR_SL1_XOR_0x71_##n, KECCAK_C1_##n,  \
      KECCAK_I_##n, KECCAK_XOR_##n

  keccak_vars(0),
  keccak_vars(1),
  keccak_vars(3),
  keccak_vars(7),
  keccak_vars(15),
  keccak_vars(31),
  keccak_vars(63),

  PTR_VAR_ROUND,
  VAR_ROUND,
  VAR_ROUND_INC,
  VAR_LOOP_COND,
  BOUND
};

// clang-format off
const uint32_t cryptonight_keccak_shader[] = {
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
  (6 << 16) | OP_EXECUTION_MODE, FUNC_MAIN, EXEC_MODE_LOCALSIZE, CRYPTONIGHT_SPV_LOCAL_WG_SIZE, 1, 1,
  // DECORATIONS
  (4 << 16) | OP_DECORATE, GLOBAL_INVOCATION_ID, DECOR_BUILTIN, BUILTIN_GLOBAL_INVOCATION_ID,
  (4 << 16) | OP_DECORATE, TYPE_ARRAY_ULONG_25, DECOR_ARRAY_STRIDE, 8,
  // state buffer
  (4 << 16) | OP_DECORATE, TYPE_RT_ARRAY_ARRAY_ULONG_25, DECOR_ARRAY_STRIDE, 200,
  (3 << 16) | OP_DECORATE, TYPE_STRUCT_BUFFER, DECOR_BLOCK,
  (4 << 16) | OP_DECORATE, PTR_BUFFER, DECOR_DESCRIPTOR_SET, 0,
  (4 << 16) | OP_DECORATE, PTR_BUFFER, DECOR_BINDING, 0,
  (5 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_BUFFER, 0, DECOR_OFFSET, 0,
  // SCALAR AND VECTOR TYPES
  (2 << 16) | OP_TYPE_VOID, TYPE_VOID,                    //type: void
  (2 << 16) | OP_TYPE_BOOL, TYPE_BOOL,                    //type: bool
  (4 << 16) | OP_TYPE_INT, TYPE_UINT, 32, 0,              //type: uint
  (4 << 16) | OP_TYPE_INT, TYPE_ULONG, 64, 0,            //type: ulong
  (4 << 16) | OP_TYPE_VECTOR, TYPE_UINT3, TYPE_UINT, 3,   //type: uvec3
  // FUNCTION TYPES
  (3 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_VOID, TYPE_VOID,//type: void fn()
  (5 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_ULONG_ULONG_UINT, TYPE_ULONG, TYPE_ULONG, TYPE_UINT,  //type: ulong fn(ulong,uint)

  // CONSTANTS
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
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_17, 17, // 17U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_18, 18, // 18U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_19, 19, // 19U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_20, 20, // 20U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_21, 21, // 21U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_22, 22, // 22U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_23, 23, // 23U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_24, 24, // 24U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_25, 25, // 25U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_27, 27, // 27U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_28, 28, // 28U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_31, 31, // 31U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_36, 36, // 36U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_39, 39, // 39U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_41, 41, // 41U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_43, 43, // 43U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_44, 44, // 44U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_45, 45, // 45U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_55, 55, // 55U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_56, 56, // 56U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_61, 61, // 61U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_62, 62, // 62U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_63, 63, // 63U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_64, 64, // 64U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_0x71, 0x71, // 0x71
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_0x80, 0x80, // 0x80

  (5 << 16) | OP_CONSTANT, TYPE_ULONG, CONST_ULONG_0,  0x00000000, 0x00000000,
  (5 << 16) | OP_CONSTANT, TYPE_ULONG, CONST_ULONG_1,  0x00000001, 0x00000000,

  // ARRAY TYPES AND POINTERS
  // globalInvocationId
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_UINT_INPUT, SC_INPUT, TYPE_UINT,   //type: [Input] uint*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_UINT3_INPUT, SC_INPUT, TYPE_UINT3, //type: [Input] uint3*

  (4 << 16) | OP_TYPE_ARRAY, TYPE_ARRAY_ULONG_25, TYPE_ULONG, CONST_UINT_25, //type: ulong[25]

  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_ARRAY_ULONG_25, SC_BUFFER, TYPE_ARRAY_ULONG_25,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_ULONG_BUFFER, SC_BUFFER, TYPE_ULONG,   //type: [Buffer] ulong*
  // state buffer
  (3 << 16) | OP_TYPE_RUNTIME_ARRAY, TYPE_RT_ARRAY_ARRAY_ULONG_25, TYPE_ARRAY_ULONG_25,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_RT_ARRAY_ARRAY_ULONG_25, SC_BUFFER, TYPE_RT_ARRAY_ARRAY_ULONG_25,
  (3 << 16) | OP_TYPE_STRUCT, TYPE_STRUCT_BUFFER, TYPE_RT_ARRAY_ARRAY_ULONG_25,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BUFFER, SC_BUFFER, TYPE_STRUCT_BUFFER,

  //
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_UINT_FUNCTION, SC_FUNCTION, TYPE_UINT,   //type: [Function] uint*

  // pointer to array element
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_ULONG_FUNCTION, SC_FUNCTION, TYPE_ULONG,   //type: [Function] ulong*

  // GLOBAL VARIABLES
  (4 << 16) | OP_VARIABLE, TYPE_PTR_UINT3_INPUT, GLOBAL_INVOCATION_ID, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_BUFFER, PTR_BUFFER, SC_BUFFER,
  // ulong rotate_left_64(ulong arg, uint num_bits)
  (5 << 16) | OP_FUNCTION, TYPE_ULONG, FUNC_ROTL64, FNC_INLINE, TYPE_FUNC_ULONG_ULONG_UINT,
  (3 << 16) | OP_FUNCTION_PARAMETER, TYPE_ULONG, ROTL64_ARG,
  (3 << 16) | OP_FUNCTION_PARAMETER, TYPE_UINT, ROTL64_NUM_BITS,
  (2 << 16) | OP_LABEL, LABEL_ROTL64,
  (5 << 16) | OP_ISUB, TYPE_UINT, ROTL64_OFFSET, CONST_UINT_64, ROTL64_NUM_BITS,
  // FIXME: LLVM optimises it to intrinsic: llvm.nvvm.rotate.b64
  //        anv NVidia spirv next gen compiler fails with unhandled intrinsic error
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_ULONG, ROTL64_SL, ROTL64_ARG, ROTL64_NUM_BITS,
  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_ULONG, ROTL64_SR, ROTL64_ARG, ROTL64_OFFSET,
  (5 << 16) | OP_BITWISE_OR, TYPE_ULONG, ROTL64_RESULT, ROTL64_SL, ROTL64_SR,
//  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_ULONG, ROTL64_SR, ROTL64_ARG, ROTL64_OFFSET,
//  (7 << 16) | OP_BITFIELD_INSERT, TYPE_ULONG, ROTL64_RESULT, ROTL64_SR, ROTL64_ARG, ROTL64_NUM_BITS, ROTL64_OFFSET,
  (2 << 16) | OP_RETURN_VALUE, ROTL64_RESULT,
  (1 << 16) | OP_FUNCTION_END,

  // MAIN
  (5 << 16) | OP_FUNCTION, TYPE_VOID, FUNC_MAIN, FNC_NONE, TYPE_FUNC_VOID,
  (2 << 16) | OP_LABEL, LABEL_MAIN,

  // round variable
  (5 << 16) | OP_VARIABLE, TYPE_PTR_UINT_FUNCTION, PTR_VAR_ROUND, SC_FUNCTION, CONST_UINT_0,
  // LFSR to calculate keccak round constants on the fly
  (5 << 16) | OP_VARIABLE, TYPE_PTR_UINT_FUNCTION, PTR_LFSR, SC_FUNCTION, CONST_UINT_1,
  // get global invocation index
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_UINT_INPUT, PTR_GLOBAL_INVOCATION_X, GLOBAL_INVOCATION_ID, CONST_UINT_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, GLOBAL_INVOCATION_X, PTR_GLOBAL_INVOCATION_X,
  // get pointer to HASH_STATE array for current invocation
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_RT_ARRAY_ARRAY_ULONG_25, PTR_BUFFER_INV, PTR_BUFFER, CONST_UINT_0,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_ARRAY_ULONG_25, PTR_HASH_STATE, PTR_BUFFER_INV, GLOBAL_INVOCATION_X,

  (2 << 16) | OP_BRANCH, LABEL_LOOP,

  // 24 rounds loop: for(int round = 0; round < 24; ++round)
  (2 << 16) | OP_LABEL, LABEL_LOOP,
  (4 << 16) | OP_LOOP_MERGE, LABEL_LOOP_END, LABEL_LOOP_INC, LC_NONE,
  (2 << 16) | OP_BRANCH, LABEL_LOOP_COND,
  (2 << 16) | OP_LABEL, LABEL_LOOP_COND,
  (4 << 16) | OP_LOAD, TYPE_UINT, VAR_ROUND, PTR_VAR_ROUND,
  (5 << 16) | OP_ULESS_THAN, TYPE_BOOL, VAR_LOOP_COND, VAR_ROUND, CONST_UINT_24, // round < 24 ?
  (4 << 16) | OP_BRANCH_CONDITIONAL, VAR_LOOP_COND, LABEL_LOOP_BODY, LABEL_LOOP_END,
  (2 << 16) | OP_LABEL, LABEL_LOOP_BODY,

  // THETA
#define load_state(x) \
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_ULONG_BUFFER, PTR_HASH_STATE_i##x, PTR_HASH_STATE, CONST_UINT_##x,\
  (4 << 16) | OP_LOAD, TYPE_ULONG, HASH_STATE_i##x, PTR_HASH_STATE_i##x
  // b0 = s[0] ^ s[5] ^ s[10] ^ s[15] ^ s[20]
  load_state(0),
  load_state(5),
  load_state(10),
  load_state(15),
  load_state(20),
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B0_0, HASH_STATE_i0, HASH_STATE_i5,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B0_1, VAR_B0_0, HASH_STATE_i10,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B0_2, VAR_B0_1, HASH_STATE_i15,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B0, VAR_B0_2, HASH_STATE_i20,
  // b1 = s[1] ^ s[6] ^ s[11] ^ s[16] ^ s[21]
  load_state(1),
  load_state(6),
  load_state(11),
  load_state(16),
  load_state(21),
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B1_0, HASH_STATE_i1, HASH_STATE_i6,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B1_1, VAR_B1_0, HASH_STATE_i11,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B1_2, VAR_B1_1, HASH_STATE_i16,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B1, VAR_B1_2, HASH_STATE_i21,
  // b2 = s[2] ^ s[7] ^ s[12] ^ s[17] ^ s[22]
  load_state(2),
  load_state(7),
  load_state(12),
  load_state(17),
  load_state(22),
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B2_0, HASH_STATE_i2, HASH_STATE_i7,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B2_1, VAR_B2_0, HASH_STATE_i12,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B2_2, VAR_B2_1, HASH_STATE_i17,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B2, VAR_B2_2, HASH_STATE_i22,
  // b3 = s[3] ^ s[8] ^ s[13] ^ s[18] ^ s[23]
  load_state(3),
  load_state(8),
  load_state(13),
  load_state(18),
  load_state(23),
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B3_0, HASH_STATE_i3, HASH_STATE_i8,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B3_1, VAR_B3_0, HASH_STATE_i13,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B3_2, VAR_B3_1, HASH_STATE_i18,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B3, VAR_B3_2, HASH_STATE_i23,
  // b4 = s[4] ^ s[9] ^ s[14] ^ s[19] ^ s[24]
  load_state(4),
  load_state(9),
  load_state(14),
  load_state(19),
  load_state(24),
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B4_0, HASH_STATE_i4, HASH_STATE_i9,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B4_1, VAR_B4_0, HASH_STATE_i14,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B4_2, VAR_B4_1, HASH_STATE_i19,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_B4, VAR_B4_2, HASH_STATE_i24,

  // BC0 = B0 ^ ROTL64(B2)
  (6 << 16) | OP_FUNCTION_CALL, TYPE_ULONG, VAR_B2_RL1, FUNC_ROTL64, VAR_B2, CONST_UINT_1,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_BC0, VAR_B0, VAR_B2_RL1,

  // BC1 = B1 ^ ROTL(B3)
  (6 << 16) | OP_FUNCTION_CALL, TYPE_ULONG, VAR_B3_RL1, FUNC_ROTL64, VAR_B3, CONST_UINT_1,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_BC1, VAR_B1, VAR_B3_RL1,

  // BC2 = B2 ^ ROTL64(B4)
  (6 << 16) | OP_FUNCTION_CALL, TYPE_ULONG, VAR_B4_RL1, FUNC_ROTL64, VAR_B4, CONST_UINT_1,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_BC2, VAR_B2, VAR_B4_RL1,

  // BC3 = B3 ^ ROTL64(B0)
  (6 << 16) | OP_FUNCTION_CALL, TYPE_ULONG, VAR_B0_RL1, FUNC_ROTL64, VAR_B0, CONST_UINT_1,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_BC3, VAR_B3, VAR_B0_RL1,

  // BC4 = B4 ^ ROTL(B1)
  (6 << 16) | OP_FUNCTION_CALL, TYPE_ULONG, VAR_B1_RL1, FUNC_ROTL64, VAR_B1, CONST_UINT_1,
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_BC4, VAR_B4, VAR_B1_RL1,

#define theta_xor(x,y)\
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_THETA_##x, VAR_BC##y, HASH_STATE_i##x

  // st[0,5,10,15,20] ^= BC4
  theta_xor(0,4),
  theta_xor(5,4),
  theta_xor(10,4),
  theta_xor(15,4),
  theta_xor(20,4),

  // st[1,6,11,16,21] ^= BC0
  theta_xor(1,0),
  theta_xor(6,0),
  theta_xor(11,0),
  theta_xor(16,0),
  theta_xor(21,0),

  // st[2,7,12,17,22] ^= BC1
  theta_xor(2,1),
  theta_xor(7,1),
  theta_xor(12,1),
  theta_xor(17,1),
  theta_xor(22,1),

  // st[3,8,13,18,23] ^= BC2
  theta_xor(3,2),
  theta_xor(8,2),
  theta_xor(13,2),
  theta_xor(18,2),
  theta_xor(23,2),

  // st[4,9,14,19,24] ^= BC3
  theta_xor(4,3),
  theta_xor(9,3),
  theta_xor(14,3),
  theta_xor(19,3),
  theta_xor(24,3),

  // END: THETA

  // RHO and PHI
#define rho(d,s,n)                                                    \
  (6 << 16) | OP_FUNCTION_CALL, TYPE_ULONG, VAR_RHO_##d, FUNC_ROTL64, VAR_THETA_##s, CONST_UINT_##n

  // identity rho(st[0]) => st[0]
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_ULONG, VAR_RHO_0, VAR_THETA_0, CONST_UINT_0,

  rho(1,6,44),
  rho(2,12,43),
  rho(3,18,21),
  rho(4,24,14),

  rho(5,3,28),
  rho(6,9,20),
  rho(7,10,3),
  rho(8,16,45),
  rho(9,22,61),

  rho(10,1,1),
  rho(11,7,6),
  rho(12,13,25),
  rho(13,19,8),
  rho(14,20,18),

  rho(15,4,27),
  rho(16,5,36),
  rho(17,11,10),
  rho(18,17,15),
  rho(19,23,56),

  rho(20,2,62),
  rho(21,8,55),
  rho(22,14,39),
  rho(23,15,41),
  rho(24,21,2),


  // END: RHO and PHI

  // CHI
  // st[i] = ~st[i + 1] & st[i + 2]
#define chi(x,y,z)                                                  \
  (4 << 16) | OP_NOT, TYPE_ULONG, VAR_NOT_RHO_##y, VAR_RHO_##y,       \
  (5 << 16) | OP_BITWISE_AND, TYPE_ULONG, VAR_CHI_S_##x, VAR_NOT_RHO_##y, VAR_RHO_##z, \
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_CHI_##x, VAR_CHI_S_##x, VAR_RHO_##x, \
  (3 << 16) | OP_STORE, PTR_HASH_STATE_i##x, VAR_CHI_##x

  chi(0,1,2),
  chi(1,2,3),
  chi(2,3,4),
  chi(3,4,0),
  chi(4,0,1),

  chi(5,6,7),
  chi(6,7,8),
  chi(7,8,9),
  chi(8,9,5),
  chi(9,5,6),

  chi(10,11,12),
  chi(11,12,13),
  chi(12,13,14),
  chi(13,14,10),
  chi(14,10,11),

  chi(15,16,17),
  chi(16,17,18),
  chi(17,18,19),
  chi(18,19,15),
  chi(19,15,16),

  chi(20,21,22),
  chi(21,22,23),
  chi(22,23,24),
  chi(23,24,20),
  chi(24,20,21),

  // END CHI

  // calculate keccak round constantusing LFSR
  (4 << 16) | OP_LOAD, TYPE_UINT, LFSR_S, PTR_LFSR,
#define KECCAK_I_S CONST_ULONG_0
#define update_keccak_roundc(n, p)                                        \
  /** v = lfsr & 0x01 ? v << n : v   */ \
  (5 << 16) | OP_BITWISE_AND, TYPE_UINT, LFSR_BIT_0_##n, LFSR_##p, CONST_UINT_1, \
  (5 << 16) | OP_INOTEQUAL, TYPE_BOOL, LFSR_BIT_0_SET_##n, LFSR_BIT_0_##n, CONST_UINT_0, \
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_ULONG, KECCAK_C1_##n, CONST_ULONG_1, CONST_UINT_##n, \
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, KECCAK_XOR_##n, KECCAK_I_##p, KECCAK_C1_##n, \
  (6 << 16) | OP_SELECT, TYPE_ULONG, KECCAK_I_##n, LFSR_BIT_0_SET_##n, KECCAK_XOR_##n, KECCAK_I_##p, \
  /** Update LFSR: lfsr = lfsr & 0x80 ? (lfsr << 1) ^ 0x71 : (lfsr << 1)  */ \
  (5 << 16) | OP_BITWISE_AND, TYPE_UINT, LFSR_AND_0x80_##n, LFSR_##p, CONST_UINT_0x80, \
  (5 << 16) | OP_INOTEQUAL, TYPE_BOOL, LFSR_BIT_7_SET_##n, LFSR_AND_0x80_##n, CONST_UINT_0, \
  (5 << 16) | OP_SHIFT_LEFT_LOGICAL, TYPE_UINT, LFSR_SL1_##n, LFSR_##p, CONST_UINT_1, \
  (5 << 16) | OP_BITWISE_XOR, TYPE_UINT, LFSR_SL1_XOR_0x71_##n, LFSR_SL1_##n, CONST_UINT_0x71, \
  (6 << 16) | OP_SELECT, TYPE_UINT, LFSR_##n, LFSR_BIT_7_SET_##n, LFSR_SL1_XOR_0x71_##n, LFSR_SL1_##n

  // 7 iterations for bitPosition [0,1,3,7,15,31,63]
  update_keccak_roundc(0, S),
  update_keccak_roundc(1, 0),
  update_keccak_roundc(3, 1),
  update_keccak_roundc(7, 3),
  update_keccak_roundc(15, 7),
  update_keccak_roundc(31, 15),
  update_keccak_roundc(63, 31),

  // store LFSR
  (3 << 16) | OP_STORE, PTR_LFSR, LFSR_63,

  // IOTA: st[0] ^= keccak_rndc[round];
  (5 << 16) | OP_BITWISE_XOR, TYPE_ULONG, VAR_IOTA_0, VAR_CHI_0, KECCAK_I_63,
  (3 << 16) | OP_STORE, PTR_HASH_STATE_i0, VAR_IOTA_0,

  (2 << 16) | OP_BRANCH, LABEL_LOOP_INC,
  (2 << 16) | OP_LABEL, LABEL_LOOP_INC,
  (5 << 16) | OP_IADD, TYPE_UINT, VAR_ROUND_INC, VAR_ROUND, CONST_UINT_1, // ; ++round
  (3 << 16) | OP_STORE, PTR_VAR_ROUND, VAR_ROUND_INC,

  (2 << 16) | OP_BRANCH, LABEL_LOOP,
  // main:return
  (2 << 16) | OP_LABEL, LABEL_LOOP_END,
  (1 << 16) | OP_RETURN,
  (1 << 16) | OP_FUNCTION_END,

};

// clang-format on

const size_t cryptonight_keccak_shader_size = sizeof(cryptonight_keccak_shader);
