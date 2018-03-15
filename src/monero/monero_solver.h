#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "monero/monero.h"
#include "monero/monero_config.h"

struct monero_solution {
  int job_id;
  uint32_t nonce;
  uint8_t hash[MONERO_OUTPUT_HASH_LEN];
};

static inline uint64_t monero_solution_hash_val(const uint8_t *hash)
{
  return *(uint64_t *)&hash[24];
}

struct monero_solver_metrics {
  uint64_t hashes_processed_total;
  uint64_t solutions_found;
  uint64_t top_10_solutions[10];
};


typedef void (*monero_solver_submit)(int solver_id,
                                     struct monero_solution *solution,
                                     void *data);

struct monero_solver_internal;
struct monero_solver {
  struct monero_solver_internal *internal;
  int solver_id;
  bool (*set_job)(struct monero_solver *, const uint8_t *input_hash,
                  size_t input_hash_len, const uint64_t target,
                  uint8_t *output_hash, uint32_t *output_nonces,
                  size_t *output_num);

  int (*process)(struct monero_solver *, uint32_t nonce_from);

  void (*free)(struct monero_solver *);
};

void monero_solver_work(struct monero_solver *ptr, monero_solver_submit submit,
                        void *submit_data, int job_id,
                        const uint8_t *input_hash, size_t input_hash_len,
                        uint64_t target, uint32_t nonce_from,
                        uint32_t nonce_to);

void monero_solver_get_metrics(struct monero_solver *,
                               struct monero_solver_metrics *);

/** new monero cpu solver */
struct monero_solver *
monero_solver_new_cpu(const struct monero_config_solver_cpu *cfg);

/** new monero opencl solver */
struct monero_solver *
monero_solver_new_cl(const struct monero_config_solver_cl *cfg);

/** new monero vulkan solver */
struct monero_solver *
monero_solver_new_vk(const struct monero_config_solver_vk *cfg);

bool monero_solver_init(const struct monero_config_solver *,
                        struct monero_solver *);
