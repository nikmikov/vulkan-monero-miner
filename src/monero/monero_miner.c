#include "monero/monero_miner.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uv.h>

#include "logging.h"
#include "monero/monero.h"
#include "monero/monero_job.h"
#include "monero/monero_result.h"
#include "monero/monero_solver.h"

#include "utils/byteswap.h"
#include "utils/hex.h"

struct monero_miner {
  struct miner miner;

  /** Solvers */
  size_t solvers_len;
  struct monero_solver **solvers;
  uint32_t nonce_chunk_size;

  /** current job */
  int job_seq_id; // internal monotonically increasing job id
  const char *job_id;
  uint64_t target;
  struct miner_event_handler *event_handler;

  // metrics
  uv_timer_t timer_req;
  time_t time_start;
  uint64_t *hashes_prev; // len == solvers_len
};

#define PRINT_METRICS_SEC 10

inline static uint64_t target_to_difficulty(uint64_t t)
{
  return t > 0 ? 0xffffffffffffffff / t : 0;
}

inline static uint64_t read_target(const char *str)
{
  char *endptr = NULL;
  uint64_t val = strtoull(str, &endptr, 16);
  if (*endptr == '\0') {
    return bswap_64(val);
  }
  return 0; // error
}

void monero_miner_print_metrics(uv_timer_t *handle)
{
  struct monero_miner *miner = handle->data;
  assert(miner != NULL);
  int seconds_elapsed = (int)difftime(time(NULL), miner->time_start);
  assert(seconds_elapsed >= 0);
  char buf[1024];
  char *buf_ptr = stpcpy(buf, "Metrics Cur:Avg:Sol ");
  struct monero_solver_metrics metrics;
  for (size_t i = 0; i < miner->solvers_len; ++i) {
    monero_solver_get_metrics(miner->solvers[i], &metrics);
    uint64_t cur = (metrics.hashes_processed_total - miner->hashes_prev[i]) /
                   PRINT_METRICS_SEC;
    miner->hashes_prev[i] = metrics.hashes_processed_total;
    uint64_t avg = metrics.hashes_processed_total / seconds_elapsed;
    buf_ptr += sprintf(buf_ptr, "| %llu:%llu:%llu ", cur, avg,
                       metrics.solutions_found);
  }
  *buf_ptr = 0;
  log_info(buf);
}

void monero_miner_submit(int solver_id, struct monero_solution *solution,
                         void *data)
{
  struct monero_miner *miner = (struct monero_miner *)data;
  if (solution->job_id != miner->job_seq_id) {
    log_warn("Stale solution detected!");
    return;
  }

  log_info("#%d: Solution found: nonce: %x, solution: %lx, target: %lx",
           solver_id, solution->nonce, monero_solution_hash_val(solution->hash),
           miner->target);
  struct monero_result result = {0, .nonce = 0};
  result.job_id = miner->job_id;
  result.nonce = solution->nonce;
  hex_from_binary(solution->hash, MONERO_OUTPUT_HASH_LEN, result.hash);

  struct miner_event_result_found result_found_evt;
  result_found_evt.event_type = MINER_EVENT_RESULT_FOUND;
  result_found_evt.data = &result;
  if (miner->event_handler != NULL) {
    miner->event_handler->cb((struct miner_event *)&result_found_evt,
                             miner->event_handler->data);
  } else {
    log_warn("Solution NOT submitted. Probably running in benchmark mode");
  }
}

void monero_miner_free(miner_handle *handle)
{
  struct monero_miner *miner = (struct monero_miner *)*handle;
  uv_timer_stop(&miner->timer_req);
  if (miner->job_id != NULL) {
    free((void *)miner->job_id);
  }
  if (miner->solvers_len > 0) {
    free(miner->hashes_prev);
    for (size_t i = 0; i < miner->solvers_len; ++i) {
      struct monero_solver *solver = miner->solvers[i];
      if (solver != NULL) {
        solver->free(solver);
      }
    }
    free(miner->solvers);
  }
  free((void *)miner);
  *handle = NULL;
}

void monero_miner_new_job(miner_handle handle, void *job_data,
                          struct miner_event_handler *event_handler)
{
  struct monero_miner *miner = (struct monero_miner *)handle;
  assert(miner->solvers_len > 0 &&
         "Should have at least one solver configured");

  struct monero_job *job = (struct monero_job *)job_data;
  log_debug("New job: {id: %s, blob: %s, target: %s}", job->job_id, job->blob,
            job->target);

  assert(job->target != NULL && strlen(job->target) > 0);
  // convert target: 64bit LE integer encoded in hex
  uint64_t target = read_target(job->target);
  if (target == 0) {
    log_error("Target is not a valid hex string: %s", job->target);
    return;
  }
  if (miner->target != target) {
    log_info("Difficulty adjusted %lu -> %lu",
             target_to_difficulty(miner->target), target_to_difficulty(target));
    miner->target = target;
  }
  // convert blob from hex to binary
  size_t blob_len = strlen(job->blob);
  uint8_t *input_hash = calloc(1, 1 + blob_len / 2);
  size_t input_hash_len = hex_to_binary(job->blob, blob_len, input_hash);
  if (input_hash_len > MONERO_INPUT_HASH_LEN) {
    log_error("Input hash length(%lu)is too large", input_hash_len);
    goto FREE;
  }
  if (input_hash_len < blob_len / 2) {
    log_error("Invalid blob: %s. Error when parsing byte #%lu", job->blob,
              input_hash_len);
    goto FREE;
  }

  // exec job
  if (miner->job_id != NULL) {
    free((void *)miner->job_id);
  }
  miner->job_id = strdup(job->job_id);
  ++miner->job_seq_id;
  miner->event_handler = event_handler;
  // submit to executors.
  // pick random starting point
  uint32_t nonce_from = ((uint64_t)rand()) % miner->nonce_chunk_size;
  log_debug("Work: %lu. Starting nonce: %x, chunk size: %u", miner->job_seq_id,
            nonce_from, miner->nonce_chunk_size);
  // split nonce into work chunks of equal size
  for (size_t i = 0; i < miner->solvers_len; ++i) {
    uint32_t nonce_to = nonce_from + miner->nonce_chunk_size;
    monero_solver_work(miner->solvers[i], monero_miner_submit, miner,
                       miner->job_seq_id, input_hash, input_hash_len,
                       miner->target, nonce_from, nonce_to);
    nonce_from = nonce_to;
  }
FREE:
  free(input_hash);
}

void monero_miner_benchmark(miner_handle h)
{
  assert(h != NULL);
  struct monero_job *job_data = monero_job_gen_random();
  h->new_job(h, job_data, NULL);
  monero_job_free(job_data);
}

miner_handle monero_miner_new(const struct monero_config *cfg)
{
  assert(cfg != NULL);
  struct monero_miner *monero_miner = calloc(1, sizeof(struct monero_miner));
  miner_handle miner = &monero_miner->miner;

  miner->new_job = monero_miner_new_job;
  miner->free = monero_miner_free;
  miner->benchmark = monero_miner_benchmark;

  size_t solvers_len = 0;
  struct monero_config_solver *p = cfg->solvers_list;
  for (; p != NULL; p = p->next, ++solvers_len)
    ;

  monero_miner->solvers_len = solvers_len;
  monero_miner->nonce_chunk_size =
      0xffffffff / (uint32_t)(monero_miner->solvers_len + 1);
  monero_miner->solvers = calloc(solvers_len, sizeof(struct monero_solver **));
  p = cfg->solvers_list;
  for (size_t i = 0; i < monero_miner->solvers_len; ++i, p = p->next) {
    switch (p->solver_type) {
    case MONERO_CONFIG_SOLVER_CPU:
      monero_miner->solvers[i] =
          monero_solver_new_cpu((const struct monero_config_solver_cpu *)p);
      break;
    case MONERO_CONFIG_SOLVER_CL:
      monero_miner->solvers[i] =
          monero_solver_new_cl((const struct monero_config_solver_cl *)p);
      break;
    case MONERO_CONFIG_SOLVER_CUDA:
      log_error("Monero CUDA solver is not suported yet");
      goto ERROR;
    }
    if (monero_miner->solvers[i] == NULL) {
      goto ERROR;
    }
    monero_miner->solvers[i]->solver_id = (int)i;
  }

  monero_miner->hashes_prev =
      calloc(sizeof(uint64_t), monero_miner->solvers_len);
  monero_miner->time_start = time(NULL);
  uv_timer_init(uv_default_loop(), &monero_miner->timer_req);
  monero_miner->timer_req.data = monero_miner;
  uv_timer_start(&monero_miner->timer_req, monero_miner_print_metrics, 5000,
                 PRINT_METRICS_SEC * 1000);

  return miner;
ERROR:
  monero_miner_free(&miner);
  return NULL;
}
