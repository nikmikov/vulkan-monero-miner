#include "utils/spirv.h"

#include "crypto/cryptonight_spv.h"

#include "crypto/aes_spv.h"

enum { // variables
  EXT_INST_GLSL_STD_450,
  FUNC_MAIN,
  LABEL_MAIN,

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
  (11 << 16)| OP_EXTENSION, LIT_SPV_KHR_storage_buffer_storage_class,
  (6 << 16) | OP_EXT_INST_IMPORT, EXT_INST_GLSL_STD_450, LIT_GLSL_std_450,
  (3 << 16) | OP_MEMORY_MODEL, AM_LOGICAL, MM_GLSL450,
  (5 << 16) | OP_ENTRY_POINT, EXEC_MODEL_GLCOMPUTE, FUNC_MAIN, LIT_MAIN,
  (6 << 16) | OP_EXECUTION_MODE, FUNC_MAIN, EXEC_MODE_LOCALSIZE, 8, 1, 1,
};

// clang-format on

const size_t cryptonight_memloop_shader_size =
    sizeof(cryptonight_memloop_shader);
