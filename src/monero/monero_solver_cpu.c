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
#include "utils/affinity.h"

#define SOLUTIONS_BUFFER_SIZE 32

struct monero_solver_cpu {
  struct monero_solver solver;

  /** set to false to terminate worker thread */
  atomic_bool is_alive;

  /** worker thread */
  uv_thread_t worker;

  /** cryptonight context */
  struct cryptonight_ctx *cryptonight_ctx;

  /** current job */
  atomic_int job_id;
  uint8_t input_hash[MONERO_INPUT_HASH_MAX_LEN];
  size_t input_hash_len;
  uint64_t target;
  uint32_t nonce_from;
  uint32_t nonce_to;
  // submit callback
  monero_solver_submit submit;
  void *submit_data;

  /** solution data */
  uv_async_t solution_found_async; // async handle on solution found
  uv_mutex_t solution_lock;
  struct monero_solver_solution solutions[SOLUTIONS_BUFFER_SIZE];
  size_t num_solutions;

  // quick metrics
  uv_timer_t timer_req;
  atomic_int hashes_processed;
  int hashes_prev;
};

void monero_solver_metrics_callback(uv_timer_t *handle)
{
  struct monero_solver_cpu *solver = handle->data;
  int hashes_processed = atomic_load(&solver->hashes_processed);
  int hashrate = hashes_processed - solver->hashes_prev;
  solver->hashes_prev = hashes_processed;
  log_info("Hashrate: %dH/sec", hashrate / 5);
}

/** Called from worker thread on main loop when solution found */
void monero_solver_cpu_solution_found(uv_async_t *handle)
{
  log_debug("Processing solution");
  assert(handle->data);
  struct monero_solver_cpu *solver = (struct monero_solver_cpu *)handle->data;
  struct monero_solver_solution solutions[solver->num_solutions];
  size_t num_solutions = 0;
  uv_mutex_lock(&solver->solution_lock);
  assert(solver->num_solutions > 0);
  // copy solutions buffer
  num_solutions = solver->num_solutions;
  memcpy(solutions, solver->solutions,
         sizeof(struct monero_solver_solution) * num_solutions);
  solver->num_solutions = 0;
  uv_mutex_unlock(&solver->solution_lock);

  // submit solution
  assert(solver->submit != NULL);
  for (size_t i = 0; i < num_solutions; ++i) {
    solver->submit(&solutions[i], solver->submit_data);
  }
}

void monero_solver_cpu_work_thread(void *arg)
{
  log_debug("Worker thread started");
  struct monero_solver_cpu *solver = arg;
  int current_job_id = 0;
  struct cryptonight_hash output_hash;
  // bytes 24..31 is what we are looking for
  uint64_t target = 0;
  uint8_t input_hash[MONERO_INPUT_HASH_MAX_LEN];
  uint32_t nonce = 0, nonce_to = 0;
  size_t input_hash_len = 0;
  uint64_t *hash_val = (uint64_t *)&output_hash.data[24];
  uint32_t *nonce_val = (uint32_t *)&input_hash[MONERO_NONCE_POSITION];
  while (atomic_load(&solver->is_alive)) {
    int j = atomic_load(&solver->job_id);
    if (j != current_job_id) {
      // load new job
      input_hash_len = solver->input_hash_len;
      assert(input_hash_len <= MONERO_INPUT_HASH_MAX_LEN);
      if (input_hash_len < MONERO_NONCE_POSITION + 4) {
        log_error("Work #%d: Invalid input hash len: %lu", input_hash_len);
        continue;
      }
      // copy hash
      memcpy(input_hash, solver->input_hash, input_hash_len);
      // nonces
      nonce = solver->nonce_from;
      nonce_to = solver->nonce_to;
      // target
      target = solver->target;
      current_job_id = j;
    } else if (nonce < nonce_to) { // work job
      // insert 4-byte nonce into job blob @ bytes 39..42
      *nonce_val = nonce;
      // run cryptonight
      cryptonight_aesni(input_hash, input_hash_len, &output_hash,
                        solver->cryptonight_ctx);
      // check target
      if (*hash_val < target) {
        // solution found
        log_info("Solution found: nonce: %x, solution: %lx, target: %lx", nonce,
                 *hash_val, target);
        uv_mutex_lock(&solver->solution_lock);
        if (solver->num_solutions < SOLUTIONS_BUFFER_SIZE) {
          struct monero_solver_solution *sol =
              &solver->solutions[solver->num_solutions++];
          sol->job_id = current_job_id;
          sol->nonce = nonce;
          memcpy(sol->hash, output_hash.data, MONERO_OUTPUT_HASH_LEN);
        } else {
          log_error("Solutions buffer full!");
        }
        uv_mutex_unlock(&solver->solution_lock);
        uv_async_send(&solver->solution_found_async); // notify main loop
      }
      atomic_fetch_add(&solver->hashes_processed, 1);
      ++nonce;
    } else {
      log_debug("No work available. Z-z-z-z...");
      sleep(1);
    }
  }
  log_debug("Worker thread quit");
}

void monero_solver_cpu_work(struct monero_solver *ptr,
                            monero_solver_submit submit, void *submit_data,
                            int job_id, const uint8_t *input_hash,
                            size_t input_hash_len, uint64_t target,
                            uint32_t nonce_from, uint32_t nonce_to)
{
  assert(ptr != NULL);
  assert(submit != NULL);
  assert(submit_data != NULL);
  assert(input_hash != NULL);
  struct monero_solver_cpu *solver = (struct monero_solver_cpu *)ptr;
  assert(input_hash_len <= MONERO_INPUT_HASH_MAX_LEN);
  log_debug("New work: %lu, target: %lx, nonce: %x - %x, %u hashes to go",
            job_id, target, nonce_from, nonce_to, (nonce_to - nonce_from));
  solver->submit = submit;
  solver->submit_data = submit_data;
  memcpy(solver->input_hash, input_hash, input_hash_len);
  solver->input_hash_len = input_hash_len;
  solver->target = target;
  solver->nonce_from = nonce_from;
  solver->nonce_to = nonce_to;

  atomic_store(&solver->job_id, job_id); // signal worker of job change
}

void monero_solver_cpu_free(struct monero_solver *ptr)
{
  struct monero_solver_cpu *solver = (struct monero_solver_cpu *)ptr;
  atomic_store(&solver->is_alive, false);

  uv_thread_join(&solver->worker);
  uv_close((uv_handle_t *)&solver->solution_found_async, NULL);
  uv_mutex_destroy(&solver->solution_lock);
  cryptonight_ctx_free(&solver->cryptonight_ctx);

  free(solver);
}

struct monero_solver *
monero_solver_new_cpu(const struct monero_config_solver_cpu *cfg)
{
  assert(cfg != NULL);

  struct monero_solver_cpu *solver_cpu =
      calloc(1, sizeof(struct monero_solver_cpu));
  atomic_store(&solver_cpu->job_id, 0);
  atomic_store(&solver_cpu->is_alive, true);
  solver_cpu->solver.work = monero_solver_cpu_work;
  solver_cpu->solver.free = monero_solver_cpu_free;

  solver_cpu->cryptonight_ctx = cryptonight_ctx_new();
  uv_mutex_init(&solver_cpu->solution_lock);
  uv_async_init(uv_default_loop(), &solver_cpu->solution_found_async,
                monero_solver_cpu_solution_found);
  solver_cpu->solution_found_async.data = solver_cpu;
  uv_thread_create(&solver_cpu->worker, monero_solver_cpu_work_thread,
                   solver_cpu);

  int affinity = cfg->solver.affine_to_cpu;
  if (affinity >= 0) {
    bool r = uv_thread_set_affinity(solver_cpu->worker, (uint64_t)affinity);
    if (!r) {
      log_warn("CPU affinity not set");
    } else {
      log_info("Set CPU affinity: %d", affinity);
    }
  }

  atomic_store(&solver_cpu->hashes_processed, 0);
  uv_timer_init(uv_default_loop(), &solver_cpu->timer_req);
  solver_cpu->timer_req.data = solver_cpu;
  uv_timer_start(&solver_cpu->timer_req, *monero_solver_metrics_callback, 5000,
                 5000);
  return (struct monero_solver *)solver_cpu;
}
