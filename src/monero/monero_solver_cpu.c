#include "monero/monero_solver.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

#include "crypto/cryptonight/cryptonight.h"
#include "logging.h"
#include "monero/monero_config.h"
#include "utils/unused.h"

#define SOLUTIONS_BUFFER_SIZE 32

struct monero_solver_cpu {
  struct monero_solver solver;

  /** cryptonight context */
  struct cryptonight_ctx *cryptonight_ctx;
  struct cryptonight_hash cryptonight_output_hash;

  /** current job */
  const uint8_t *input_hash;
  size_t input_hash_len;
  uint32_t *nonce_ptr;
  uint64_t target;
  uint8_t *output_hash;
  uint32_t *output_nonces;
  size_t *output_num;
};

bool monero_solver_cpu_set_job(struct monero_solver *ptr,
                               const uint8_t *input_hash, size_t input_hash_len,
                               const uint64_t target, uint8_t *output_hash,
                               uint32_t *output_nonces, size_t *output_num)
{
  struct monero_solver_cpu *solver = (struct monero_solver_cpu *)ptr;
  solver->input_hash = input_hash;
  solver->input_hash_len = input_hash_len;
  solver->target = target;
  solver->nonce_ptr = (uint32_t *)&solver->input_hash[MONERO_NONCE_POSITION];
  solver->output_hash = output_hash;
  solver->output_nonces = output_nonces;
  solver->output_num = output_num;
  return true;
}

// *output_hash: SOLUTIONS_BUFFER_SIZE * MONERO_OUTPUT_HASH_LEN
int monero_solver_cpu_process(struct monero_solver *ptr, uint32_t nonce_from)
{
  struct monero_solver_cpu *solver = (struct monero_solver_cpu *)ptr;

  *solver->nonce_ptr = nonce_from;
  cryptonight_aesni(solver->input_hash, solver->input_hash_len,
                    &solver->cryptonight_output_hash, solver->cryptonight_ctx);

  if (monero_solution_hash_val(solver->cryptonight_output_hash.data) <
      solver->target) {
    log_debug("Solution found: %x!", *solver->nonce_ptr);
    // solution found
    memcpy(solver->output_hash, solver->cryptonight_output_hash.data,
           MONERO_OUTPUT_HASH_LEN);
    *solver->output_nonces = nonce_from;
    *solver->output_num = 1;
  }
  return 1;
}

void monero_solver_cpu_free(struct monero_solver *ptr)
{
  struct monero_solver_cpu *solver = (struct monero_solver_cpu *)ptr;

  cryptonight_ctx_free(&solver->cryptonight_ctx);

  free(ptr);
}

struct monero_solver *
monero_solver_new_cpu(const struct monero_config_solver_cpu *cfg)
{
  assert(cfg != NULL);

  struct monero_solver_cpu *solver_cpu =
      calloc(1, sizeof(struct monero_solver_cpu));

  solver_cpu->solver.set_job = monero_solver_cpu_set_job;
  solver_cpu->solver.process = monero_solver_cpu_process;
  solver_cpu->solver.free = monero_solver_cpu_free;

  if (monero_solver_init(&cfg->solver, &solver_cpu->solver)) {
    solver_cpu->cryptonight_ctx = cryptonight_ctx_new();
    return &solver_cpu->solver;
  } else {
    free(solver_cpu);
    return NULL;
  }
}
