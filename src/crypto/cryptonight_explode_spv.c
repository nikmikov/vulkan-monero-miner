#include "utils/spirv.h"

#include "crypto/cryptonight_spv.h"

enum { // variables
  RESERVED_ID = 0,
  EXT_INST_GLSL_STD_450,
  FUNC_MAIN,
  LABEL_MAIN,
  // global variables
  GLOBAL_INVOCATION_ID,
  // types
  TYPE_VOID,
  TYPE_FUNC_VOID,
  TYPE_BOOL,
  TYPE_UINT,
  TYPE_UINT3,
  TYPE_PTR_UINT_INPUT,
  TYPE_PTR_UINT3_INPUT,
  TYPE_ARRAY_UINT_50,
  TYPE_PTR_ARRAY_UINT_50,
  TYPE_RT_ARRAY_ARRAY_UINT_50,
  TYPE_PTR_RT_ARRAY_ARRAY_UINT_50,
  TYPE_ARRAY_UINT_2097152,
  TYPE_PTR_ARRAY_UINT_2097152,
  TYPE_RT_ARRAY_ARRAY_UINT_2097152,
  TYPE_PTR_RT_ARRAY_ARRAY_UINT_2097152,
  TYPE_STRUCT_STATE_BUFFER,
  TYPE_PTR_STATE_BUFFER,
  TYPE_STRUCT_SCRATCHPAD_BUFFER,
  TYPE_PTR_SCRATCHPAD_BUFFER,
  // pointers
  PTR_GLOBAL_INVOCATION_X,
  PTR_STATE_BUFFER,
  PTR_SCRATCHPAD_BUFFER,
  // main: local variables
  GLOBAL_INVOCATION_X,

  BOUND
};

// clang-format off
const uint32_t cryptonight_scratchpad_explode_shader[] = {
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
  (4 << 16) | OP_DECORATE, GLOBAL_INVOCATION_ID, DECOR_BUILTIN, BUILTIN_GLOBAL_INVOCATION_ID,
  (4 << 16) | OP_DECORATE, TYPE_ARRAY_UINT_50, DECOR_ARRAY_STRIDE, 4,
  // state buffer
  (4 << 16) | OP_DECORATE, TYPE_RT_ARRAY_ARRAY_UINT_50, DECOR_ARRAY_STRIDE, 200,
  (3 << 16) | OP_DECORATE, TYPE_STRUCT_STATE_BUFFER, DECOR_BLOCK,
  (4 << 16) | OP_DECORATE, PTR_STATE_BUFFER, DECOR_DESCRIPTOR_SET, 0,
  (4 << 16) | OP_DECORATE, PTR_STATE_BUFFER, DECOR_BINDING, 0,
  (5 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_STATE_BUFFER, 0, DECOR_OFFSET, 0,
  // scratchpad buffer
  (4 << 16) | OP_DECORATE, TYPE_RT_ARRAY_ARRAY_UINT_2097152, DECOR_ARRAY_STRIDE, 2097152,
  (3 << 16) | OP_DECORATE, TYPE_STRUCT_SCRATCHPAD_BUFFER, DECOR_BLOCK,
  (4 << 16) | OP_DECORATE, PTR_SCRATCHPAD_BUFFER, DECOR_DESCRIPTOR_SET, 0,
  (4 << 16) | OP_DECORATE, PTR_SCRATCHPAD_BUFFER, DECOR_BINDING, 1,
  (5 << 16) | OP_MEMBER_DECORATE, TYPE_STRUCT_SCRATCHPAD_BUFFER, 0, DECOR_OFFSET, 0,

  // SCALAR AND VECTOR TYPES
  (2 << 16) | OP_TYPE_VOID, TYPE_VOID,                    //type: void
  (2 << 16) | OP_TYPE_BOOL, TYPE_BOOL,                    //type: bool
  (4 << 16) | OP_TYPE_INT, TYPE_UINT, 32, 0,              //type: uint
  (4 << 16) | OP_TYPE_VECTOR, TYPE_UINT3, TYPE_UINT, 3,   //type: uvec3
  // FUNCTION TYPES
  (3 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_VOID, TYPE_VOID,//type: void fn()

  // globalInvocationId
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_UINT_INPUT, SC_INPUT, TYPE_UINT,   //type: [Input] uint*
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_UINT3_INPUT, SC_INPUT, TYPE_UINT3, //type: [Input] uint3*
  // state buffer
  (3 << 16) | OP_TYPE_RUNTIME_ARRAY, TYPE_RT_ARRAY_ARRAY_UINT_50, TYPE_ARRAY_UINT_50,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_RT_ARRAY_ARRAY_UINT_50, SC_BUFFER, TYPE_RT_ARRAY_ARRAY_UINT_50,
  (3 << 16) | OP_TYPE_STRUCT, TYPE_STRUCT_STATE_BUFFER, TYPE_RT_ARRAY_ARRAY_UINT_50,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_STATE_BUFFER, SC_BUFFER, TYPE_STRUCT_STATE_BUFFER,
  // scratchpad buffer
  (3 << 16) | OP_TYPE_RUNTIME_ARRAY, TYPE_RT_ARRAY_ARRAY_UINT_2097152, TYPE_ARRAY_UINT_2097152,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_RT_ARRAY_ARRAY_UINT_2097152, SC_BUFFER, TYPE_RT_ARRAY_ARRAY_UINT_2097152,
  (3 << 16) | OP_TYPE_STRUCT, TYPE_STRUCT_SCRATCHPAD_BUFFER, TYPE_RT_ARRAY_ARRAY_UINT_2097152,
  (4 << 16) | OP_TYPE_POINTER, TYPE_PTR_SCRATCHPAD_BUFFER, SC_BUFFER, TYPE_STRUCT_SCRATCHPAD_BUFFER,

  // GLOBAL VARIABLES
  (4 << 16) | OP_VARIABLE, TYPE_PTR_UINT3_INPUT, GLOBAL_INVOCATION_ID, SC_INPUT,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_STATE_BUFFER, PTR_STATE_BUFFER, SC_BUFFER,
  (4 << 16) | OP_VARIABLE, TYPE_PTR_SCRATCHPAD_BUFFER, PTR_SCRATCHPAD_BUFFER, SC_BUFFER,

  // MAIN
  (5 << 16) | OP_FUNCTION, TYPE_VOID, FUNC_MAIN, FNC_NONE, TYPE_FUNC_VOID,
  (2 << 16) | OP_LABEL, LABEL_MAIN,

  // main:return
  (1 << 16) | OP_RETURN,
  (1 << 16) | OP_FUNCTION_END,
};

// clang-format on

const size_t cryptonight_scratchpad_explode_shader_size =
    sizeof(cryptonight_scratchpad_explode_shader);
