#pragma once

#include "monero/monero.h"

#include <stdint.h>

struct monero_result {
  const char *job_id;
  uint32_t nonce;
  char hash[1 + MONERO_OUTPUT_HASH_LEN * 2];
};
