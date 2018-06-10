#pragma once

#include <stddef.h>
#include <stdint.h>

#define CRYPTONIGHT_SPV_LOCAL_WG_SIZE 8

extern const uint32_t cryptonight_init_shader[];

extern const size_t cryptonight_init_shader_size;

extern const uint32_t cryptonight_keccak_shader[];

extern const size_t cryptonight_keccak_shader_size;

extern const uint32_t cryptonight_explode_shader[];

extern const size_t cryptonight_explode_shader_size;

extern const uint32_t cryptonight_memloop_shader[];

extern const size_t cryptonight_memloop_shader_size;

extern const uint32_t cryptonight_implode_shader[];

extern const size_t cryptonight_implode_shader_size;
