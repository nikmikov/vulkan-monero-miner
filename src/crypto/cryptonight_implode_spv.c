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

  // SCALAR AND VECTOR TYPES
  (2 << 16) | OP_TYPE_VOID, TYPE_VOID,                    //type: void

  // FUNCTION TYPES
  (3 << 16) | OP_TYPE_FUNCTION, TYPE_FUNC_VOID, TYPE_VOID,//type: void fn()


  // MAIN
  (5 << 16) | OP_FUNCTION, TYPE_VOID, FUNC_MAIN, FNC_NONE, TYPE_FUNC_VOID,
  (2 << 16) | OP_LABEL, LABEL_MAIN,


  // main:return
  (1 << 16) | OP_RETURN,
  (1 << 16) | OP_FUNCTION_END,

};

// clang-format on

const size_t cryptonight_implode_shader_size =
    sizeof(cryptonight_implode_shader);
