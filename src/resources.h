#pragma once

#include <stdint.h>

extern const char *RC_CL_SOURCE_CRYPTONIGHT;

// shaders
extern uint8_t spv_cn_init[] __asm("_binary_crypto_cn_init_spv_start");
extern uint8_t spv_cn_init_end[] __asm("_binary_crypto_cn_init_spv_end");

extern uint8_t spv_cn_keccak[] __asm("_binary_crypto_cn_keccak_spv_start");
extern uint8_t spv_cn_keccak_end[] __asm("_binary_crypto_cn_keccak_spv_end");
