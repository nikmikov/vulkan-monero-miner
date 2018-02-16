#include "monero/monero_solver.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

#include "logging.h"
#include "utils/affinity.h"
#include "utils/port_sleep.h"

#define SOLUTIONS_BUFFER_SIZE 256

struct monero_solver_internal {
  /** set to false to terminate worker thread */
  atomic_bool is_alive;

  /** worker thread */
  uv_thread_t worker;

  /** current job */
  atomic_int job_id;
  uint8_t input_hash[MONERO_INPUT_HASH_LEN];
  size_t input_hash_len;
  uint64_t target;
  uint32_t nonce_from;
  uint32_t nonce_to;

  /** submit callback */
  monero_solver_submit submit;
  void *submit_data;

  /** solution data */
  uv_async_t solution_found_async; // async handle on solution found
  uv_mutex_t solution_lock;
  struct monero_solution solutions[SOLUTIONS_BUFFER_SIZE];
  size_t num_solutions;

  /** quick metrics */
  struct monero_solver_metrics metrics;
  atomic_int hashes_counter;
};

#define MIN(a, b) (a < b ? a : b)

void monero_solver_get_metrics(struct monero_solver *solver,
                               struct monero_solver_metrics *metrics)
{
  assert(solver != NULL);
  assert(metrics != NULL);
  struct monero_solver_internal *s = solver->internal;

  s->metrics.hashes_processed_total += atomic_exchange(&s->hashes_counter, 0);
  *metrics = s->metrics;
}

static inline void metrics_add_solution(struct monero_solver_metrics *m,
                                        uint64_t sol)
{
  ++m->solutions_found;
  for (size_t i = 0; i < m->solutions_found && i < 10; ++i) {
    if (m->top_10_solutions[i] > sol) {
      uint64_t tmp = m->top_10_solutions[i];
      m->top_10_solutions[i] = sol;
      sol = tmp;
    }
  }
}

/** Called from worker thread on main loop when solution found */
void monero_solver_solution_found(uv_async_t *handle)
{
  assert(handle->data);
  struct monero_solver *s = (struct monero_solver *)handle->data;
  struct monero_solver_internal *solver = s->internal;

  struct monero_solution solutions[solver->num_solutions];
  size_t num_solutions = 0;
  uv_mutex_lock(&solver->solution_lock);
  assert(solver->num_solutions > 0);
  // copy solutions buffer
  num_solutions = solver->num_solutions;
  memcpy(solutions, solver->solutions,
         sizeof(struct monero_solution) * num_solutions);
  solver->num_solutions = 0;
  uv_mutex_unlock(&solver->solution_lock);

  // submit solution
  assert(solver->submit != NULL);
  for (size_t i = 0; i < num_solutions; ++i) {
    solver->submit(s->solver_id, &solutions[i], solver->submit_data);
    metrics_add_solution(&solver->metrics,
                         monero_solution_hash_val(solutions[i].hash));
  }
}

void monero_solver_work_thread(void *arg)
{
  log_debug("Worker thread started");
  struct monero_solver *s = arg;
  struct monero_solver_internal *solver = s->internal;

  int current_job_id = 0;
  uint8_t input_hash[MONERO_INPUT_HASH_LEN];
  uint32_t nonce = 0, nonce_to = 0;
  size_t input_hash_len = 0;
  uint64_t target = 0;
  uint8_t output_hash[MONERO_OUTPUT_HASH_LEN * SOLUTIONS_BUFFER_SIZE];
  bool new_job = false;
  while (atomic_load(&solver->is_alive)) {
    int j = atomic_load(&solver->job_id);
    if (j != current_job_id) {
      // LOAD NEW JOB
      nonce = nonce_to = 0;

      input_hash_len = MIN(solver->input_hash_len, MONERO_INPUT_HASH_LEN);

      if (input_hash_len < MONERO_NONCE_POSITION + 4) {
        log_error("Work #%d: Invalid input hash len: %lu", input_hash_len);
      } else {
        // copy hash
        memcpy(input_hash, solver->input_hash, input_hash_len);
        if (input_hash_len < MONERO_INPUT_HASH_LEN) {
          memcpy(input_hash + input_hash_len, 0,
                 MONERO_INPUT_HASH_LEN - input_hash_len);
        }
        // nonces
        nonce = solver->nonce_from;
        nonce_to = solver->nonce_to;
        // target
        target = solver->target;
        current_job_id = j;
      }
      new_job = true;
    } else if (nonce < nonce_to) {
      if (new_job) {
        if (s->set_job(s, input_hash, target)) {
          new_job = false;
        } else {
          // error
          log_error("Error sending job to worker");
          nonce = nonce_to;
          continue; // skip this job
        }
      }
      // PROCESS ONE CHUNK
      size_t solutions_found = 0;

      int nonces_processed =
          s->process(s, nonce, output_hash, &solutions_found);
      bool success = nonces_processed >= 0;
      if (success && solutions_found > 0) {
        uv_mutex_lock(&solver->solution_lock);
        // copy solutions
        for (size_t i = 0; i < solutions_found; ++i) {
          if (solver->num_solutions < SOLUTIONS_BUFFER_SIZE) {
            struct monero_solution *sol =
                &solver->solutions[solver->num_solutions++];
            sol->job_id = current_job_id;
            memcpy(sol->hash, &output_hash[MONERO_OUTPUT_HASH_LEN * i],
                   MONERO_OUTPUT_HASH_LEN);
          } else {
            log_error("Solutions buffer full!");
          }
        }
        uv_mutex_unlock(&solver->solution_lock);
        uv_async_send(&solver->solution_found_async); // notify main loop
      }
      if (success) {
        atomic_fetch_add(&solver->hashes_counter, nonces_processed);
        nonce += nonces_processed;
      }
    } else {
      // SLEEP: NO JOB AVAILABLE
      log_debug("No work available. Z-z-z-z...");
      sleep(1);
    }
  }
  log_debug("Worker thread quit");
}

void monero_solver_free(struct monero_solver *ptr)
{
  struct monero_solver_internal *solver = ptr->internal;
  atomic_store(&solver->is_alive, false);

  uv_thread_join(&solver->worker);
  uv_close((uv_handle_t *)&solver->solution_found_async, NULL);
  uv_mutex_destroy(&solver->solution_lock);

  free(solver);
  ptr->free(ptr);
}

bool monero_solver_init(const struct monero_config_solver *cfg,
                        struct monero_solver *s)
{
  assert(cfg != NULL);

  struct monero_solver_internal *solver =
      calloc(1, sizeof(struct monero_solver_internal));

  atomic_store(&solver->job_id, 0);
  atomic_store(&solver->is_alive, true);
  atomic_store(&solver->hashes_counter, 0);
  //  solver->solver.work = monero_solver_work;
  // solver->solver.free = monero_solver_free;
  // solver->solver.get_metrics = monero_solver_get_metrics;

  uv_mutex_init(&solver->solution_lock);
  uv_async_init(uv_default_loop(), &solver->solution_found_async,
                monero_solver_solution_found);
  solver->solution_found_async.data = solver;

  s->internal = solver;
  uv_thread_create(&solver->worker, monero_solver_work_thread, s);

  int affinity = cfg->affine_to_cpu;
  if (affinity >= 0) {
    bool r = uv_thread_set_affinity(solver->worker, (uint64_t)affinity);
    if (!r) {
      log_warn("CPU affinity not set");
    } else {
      log_info("Set CPU affinity: %d", affinity);
    }
  }

  return true;
}

void monero_solver_work(struct monero_solver *ptr, monero_solver_submit submit,
                        void *submit_data, int job_id,
                        const uint8_t *input_hash, size_t input_hash_len,
                        uint64_t target, uint32_t nonce_from, uint32_t nonce_to)
{
  assert(ptr != NULL);
  assert(submit != NULL);
  assert(submit_data != NULL);
  assert(input_hash != NULL);
  struct monero_solver_internal *solver =
      ((struct monero_solver *)ptr)->internal;

  if (input_hash_len <= MONERO_INPUT_HASH_LEN) {
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
  } else {
    assert(false);
    log_error("Programming error: received input hash is too long: %lu",
              input_hash_len);
  }
}
