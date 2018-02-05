#pragma once

#include <stddef.h>
#include <stdint.h>

#include "monero/monero.h"
#include "monero/monero_config.h"

struct monero_solver_solution {
  int job_id;
  uint32_t nonce;
  uint8_t hash[MONERO_OUTPUT_HASH_LEN];
};

typedef void (*monero_solver_submit)(struct monero_solver_solution *solution,
                                     void *data);

struct monero_solver {
  void (*work)(struct monero_solver *, monero_solver_submit submit,
               void *submit_data, int job_id, const uint8_t *input_hash,
               size_t input_hash_len, uint64_t target, uint32_t nonce_from,
               uint32_t nonce_to);
  void (*free)(struct monero_solver *);
};

/** new monero cpu kernel */
struct monero_solver *
monero_solver_new_cpu(const struct monero_config_solver_cpu *cfg);
