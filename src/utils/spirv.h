#pragma once

#define SPIRV_MAGIC 0x07230203 // magic header ID

// "GLSL.std.450" literal packed into 4 words
#define LIT_GLSL_std_450 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000
#define LIT_MAIN 0x6e69616d, 0x00000000
#define LIT_SPV_KHR_storage_buffer_storage_class                               \
  0x5f565053, 0x5f52484b, 0x726f7473, 0x5f656761, 0x66667562, 0x735f7265,      \
      0x61726f74, 0x635f6567, 0x7373616c, 0x00000000 // len: 10

enum SPV_ADDRESSING_MODEL { AM_LOGICAL = 0 };

enum SPV_MEMORY_MODEL {
  MM_SIMPLE = 0,
  MM_GLSL450 = 1
};

enum SPV_FUNCTION_CONTROL {
  FNC_NONE = 0,
  FNC_INLINE = 1,
  FNC_DONT_INLINE = 2,
  FNC_PURE = 4,
  FNC_CONST = 8
};

enum SPV_EXECUTION_MODE {
  EXEC_MODE_LOCALSIZE = 17
};

enum SPV_EXECUTION_MODEL {
  EXEC_MODEL_GLCOMPUTE = 5
};

enum SPV_STORAGE_CLASS {
  SC_UNIFORM_CONST = 0,
  SC_INPUT = 1,
  SC_UNIFORM = 2,
  SC_FUNCTION = 7,
  SC_BUFFER = 12 // storage buffer
};

enum SPV_DECORATIONS {
  DECOR_RELAXED_PRECISION = 0,
  DECOR_BLOCK = 2,
  DECOR_ARRAY_STRIDE = 6,
  DECOR_BUILTIN = 11,
  DECOR_NON_WRITABLE = 24,
  DECOR_NON_READABLE = 25,
  DECOR_BINDING = 33,
  DECOR_DESCRIPTOR_SET = 34,
  DECOR_OFFSET = 35
};

enum SPV_BUILTINS {
  BUILTIN_GLOBAL_INVOCATION_ID = 28
};

enum SPV_CAPABILITIES {
  CAP_SHADER = 1,
  CAP_ADDRESSES = 4,
  CAP_KERNEL = 6,
  CAP_INT64 = 11
};

enum SPV_LOOP_CONTROL {
  LC_NONE = 0,
  LC_UNROLL = 1,
  LC_DONT_UNROLL = 2
};

enum SPV_OPS {
  OP_EXTENSION = 10,
  OP_EXT_INST_IMPORT = 11,
  OP_MEMORY_MODEL = 14,
  OP_ENTRY_POINT = 15,
  OP_EXECUTION_MODE = 16,
  OP_CAPABILITY = 17,
  OP_TYPE_VOID = 19,
  OP_TYPE_BOOL = 20,
  OP_TYPE_INT = 21,
  OP_TYPE_VECTOR = 23,
  OP_TYPE_ARRAY = 28,
  OP_TYPE_RUNTIME_ARRAY = 29,
  OP_TYPE_STRUCT = 30,
  OP_TYPE_POINTER = 32,
  OP_TYPE_FUNCTION = 33,
  OP_CONSTANT = 43,
  OP_CONSTANT_COMPOSITE = 44,
  OP_FUNCTION = 54,
  OP_FUNCTION_PARAMETER = 55,
  OP_FUNCTION_END = 56,
  OP_FUNCTION_CALL = 57,
  OP_VARIABLE = 59,
  OP_LOAD = 61,
  OP_STORE = 62,
  OP_COPY_MEMORY = 63,
  OP_COPY_MEMORY_SIZED = 64,
  OP_ACCESS_CHAIN = 65,
  OP_DECORATE = 71,
  OP_MEMBER_DECORATE = 72,
  OP_UCONVERT = 113,
  OP_IADD = 128,
  OP_ISUB = 130,
  OP_IMUL = 132,
  OP_ULESS_THAN = 176,
  OP_SHIFT_RIGHT_LOGICAL = 194,
  OP_SHIFT_LEFT_LOGICAL = 196,
  OP_BITWISE_OR = 197,
  OP_BITWISE_XOR = 198,
  OP_BITWISE_AND = 199,
  OP_NOT = 200,
  OP_BITFIELD_INSERT = 201,
  OP_MEMORY_BARRIER = 225,
  OP_LOOP_MERGE = 246,
  OP_LABEL = 248,
  OP_BRANCH = 249,
  OP_BRANCH_CONDITIONAL = 250,
  OP_RETURN = 253,
  OP_RETURN_VALUE = 254,
};
