#include "utils/spirv.h"

#include "crypto/cryptonight_spv.h"

enum { // variables
  RESERVED_ID = 0,
  EXT_INST_GLSL_STD_450,
  FUNC_MAIN,
  LABEL_MAIN,
  // global variables
  BUFFER_STATE,
  GLOBAL_INVOCATION_ID,
  // types
  TYPE_VOID,
  TYPE_FUNC_VOID,
  TYPE_UINT,
  TYPE_UINT3,
  TYPE_ARRAY_UINT_50,
  TYPE_STRUCT_BUFFER_INPUT,
  TYPE_STRUCT_BUFFER_OUTPUT,
  TYPE_RT_ARRAY_ARRAY_UINT_50,

  TYPE_PTR_IN_UINT,
  TYPE_PTR_IN_UINT3,
  TYPE_PTR_BF_ARRAY_UINT_50,
  TYPE_PTR_BF_UINT,
  TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT_50,
  TYPE_PTR_BF_INPUT,
  TYPE_PTR_BF_OUTPUT,
  // constants
  CONST_UINT_0,
  CONST_UINT_1,
  CONST_UINT_8,
  CONST_UINT_9,
  CONST_UINT_10,
  CONST_UINT_24,
  CONST_UINT_50,
  // pointers
  PTR_GLOBAL_INVOCATION_X,
  PTR_BUFFER_INPUT,
  PTR_START_NONCE,
  PTR_INPUT,
  PTR_BUFFER_OUTPUT,
  PTR_OUTPUT,
  PTR_HASH_STATE,
  PTR_HASH_STATE_W9,
  PTR_INPUT_W9,
  PTR_HASH_STATE_W10,
  PTR_INPUT_W10,
  // local variables
  GLOBAL_INVOCATION_X,
  START_NONCE,
  NONCE,
  NONCE_SR_8,
  INPUT_W9,
  INPUT_W10,
  INPUT_W9_MOD,
  INPUT_W10_MOD,
  BOUND
};

// clang-format off
const uint32_t cryptonight_init_shader[] = {
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
  (6 << 16) | OP_EXECUTION_MODE, FUNC_MAIN, EXEC_MODE_LOCALSIZE, 1, 1, 1,

  // DECORATIONS
  (4 << 16) | OP_DECORATE, GLOBAL_INVOCATION_ID, DECOR_BUILTIN, BUILTIN_GLOBAL_INVOCATION_ID,
  // input buffer
  (4 << 16) | OP_DECORATE, TYPE_ARRAY_UINT_50, DECOR_ARRAY_STRIDE, 4,
  (3 << 16) | OP_DECORATE, TYPE_STRUCT_BUFFER_INPUT, DECOR_BLOCK,
  (4 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_BUFFER_INPUT, 0, DECOR_NON_WRITABLE,
  (5 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_BUFFER_INPUT, 0, DECOR_OFFSET, 0,
  (4 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_BUFFER_INPUT, 1, DECOR_NON_WRITABLE,
  (5 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_BUFFER_INPUT, 1, DECOR_OFFSET, 4,
  (4 << 16) | OP_DECORATE, PTR_BUFFER_INPUT, DECOR_DESCRIPTOR_SET, 0,
  (4 << 16) | OP_DECORATE, PTR_BUFFER_INPUT, DECOR_BINDING, 0,
  // output buffer
  (4 << 16) | OP_DECORATE, TYPE_RT_ARRAY_ARRAY_UINT_50, DECOR_ARRAY_STRIDE, 200,
  (3 << 16) | OP_DECORATE, TYPE_STRUCT_BUFFER_OUTPUT, DECOR_BLOCK,
  (4 << 16) | OP_DECORATE, PTR_BUFFER_OUTPUT, DECOR_DESCRIPTOR_SET, 0,
  (4 << 16) | OP_DECORATE, PTR_BUFFER_OUTPUT, DECOR_BINDING, 1,
  (5 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_BUFFER_OUTPUT, 0, DECOR_OFFSET, 0,
  (4 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_BUFFER_OUTPUT, 0, DECOR_NON_READABLE,

  // SCALAR AND VECTOR TYPES
  (2 << 16) | OP_TYPE_VOID, TYPE_VOID,                    //type: void
  (3 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_VOID, TYPE_VOID,//type: void fn()
  (4 << 16) | OP_TYPE_INT, TYPE_UINT, 32, 0,              //type: uint
  (4 << 16) | OP_TYPE_VECTOR, TYPE_UINT3, TYPE_UINT, 3,   //type: uvec3

  // CONSTANTS
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_0, 0, // 0U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_1, 1, // 1U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_8, 8, // 8U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_9, 9, // 9U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_10, 10, // 10U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_24, 24, // 24U
  (4 << 16) | OP_CONSTANT, TYPE_UINT, CONST_UINT_50, 50,  // 50U

  // ARRAY TYPES AND POINTERS
  // globalInvocationId
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_IN_UINT, SC_INPUT, TYPE_UINT,   //type: [Input] uint*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_IN_UINT3, SC_INPUT, TYPE_UINT3, //type: [Input] uint3*

  (4 << 16) | OP_TYPE_ARRAY, TYPE_ARRAY_UINT_50, TYPE_UINT, CONST_UINT_50, //type: uint[50]
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_ARRAY_UINT_50, SC_BUFFER, TYPE_ARRAY_UINT_50,
  // input buffer
  (4 << 16) | OP_TYPE_STRUCT, TYPE_STRUCT_BUFFER_INPUT, TYPE_UINT, TYPE_ARRAY_UINT_50,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_UINT, SC_BUFFER, TYPE_UINT,   //type: [Buffer] uint*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_INPUT, SC_BUFFER, TYPE_STRUCT_BUFFER_INPUT,
  // output buffer
  (3 << 16) | OP_TYPE_RUNTIME_ARRAY, TYPE_RT_ARRAY_ARRAY_UINT_50, TYPE_ARRAY_UINT_50,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT_50, SC_BUFFER, TYPE_RT_ARRAY_ARRAY_UINT_50,
  (3 << 16) | OP_TYPE_STRUCT, TYPE_STRUCT_BUFFER_OUTPUT, TYPE_RT_ARRAY_ARRAY_UINT_50,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_BF_OUTPUT, SC_BUFFER, TYPE_STRUCT_BUFFER_OUTPUT,

  // GLOBAL VARIABLES
  (4 << 16) | OP_VARIABLE, TYPE_PTR_IN_UINT3, GLOBAL_INVOCATION_ID, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_BF_INPUT, PTR_BUFFER_INPUT, SC_BUFFER,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_BF_OUTPUT, PTR_BUFFER_OUTPUT, SC_BUFFER,

  // MAIN
  (5 << 16) | OP_FUNCTION, TYPE_VOID, FUNC_MAIN, FNC_NONE, TYPE_FUNC_VOID,
  (2 << 16) | OP_LABEL, LABEL_MAIN,
  // get global invocation index
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_IN_UINT, PTR_GLOBAL_INVOCATION_X, GLOBAL_INVOCATION_ID, CONST_UINT_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, GLOBAL_INVOCATION_X, PTR_GLOBAL_INVOCATION_X,
  // get pointer to HASH_STATE array for current invocation
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_RT_ARRAY_ARRAY_UINT_50, PTR_OUTPUT, PTR_BUFFER_OUTPUT, CONST_UINT_0,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_ARRAY_UINT_50, PTR_HASH_STATE, PTR_OUTPUT, GLOBAL_INVOCATION_X,
  // pointer to input buffer
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_ARRAY_UINT_50, PTR_INPUT, PTR_BUFFER_INPUT, CONST_UINT_1,

  // copy 200 bytes of input hash to state buffer
  (3 << 16) | OP_COPY_MEMORY, PTR_HASH_STATE, PTR_INPUT,
  // get start nonce
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_START_NONCE, PTR_BUFFER_INPUT, CONST_UINT_0,
  (4 << 16) | OP_LOAD, TYPE_UINT, START_NONCE, PTR_START_NONCE,
  // calculate nonce as base_nonce + global_invocation_id
  (5 << 16) | OP_IADD, TYPE_UINT, NONCE, START_NONCE, GLOBAL_INVOCATION_X,
  // insert nonce into bytes 39..41 of input hash(words 9 and 10)
  // * 8 low bits of nonce into 8 high-bits of word 9 of input hash
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_INPUT_W9, PTR_INPUT, CONST_UINT_9,
  (4 << 16) | OP_LOAD, TYPE_UINT, INPUT_W9, PTR_INPUT_W9,
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, INPUT_W9_MOD, INPUT_W9, NONCE, CONST_UINT_24, CONST_UINT_8,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_HASH_STATE_W9, PTR_HASH_STATE, CONST_UINT_9,
  (3 << 16) | OP_STORE, PTR_HASH_STATE_W9, INPUT_W9_MOD,
  // * 24 high bits of nonce into 24 low bits of word 10 of inpuit
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_INPUT_W10, PTR_INPUT, CONST_UINT_10,
  (4 << 16) | OP_LOAD, TYPE_UINT, INPUT_W10, PTR_INPUT_W10,
  (5 << 16) | OP_SHIFT_RIGHT_LOGICAL, TYPE_UINT, NONCE_SR_8, NONCE, CONST_UINT_8,
  (7 << 16) | OP_BITFIELD_INSERT, TYPE_UINT, INPUT_W10_MOD, INPUT_W10, NONCE_SR_8, CONST_UINT_0, CONST_UINT_24,
  (5 << 16) | OP_ACCESS_CHAIN, TYPE_PTR_BF_UINT, PTR_HASH_STATE_W10, PTR_HASH_STATE, CONST_UINT_10,
  (3 << 16) | OP_STORE, PTR_HASH_STATE_W10, INPUT_W10_MOD,

  (1 << 16) | OP_RETURN,
  (1 << 16) | OP_FUNCTION_END,
};

// clang-format on

const size_t cryptonight_init_shader_size = sizeof(cryptonight_init_shader);
