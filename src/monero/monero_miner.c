#include "monero/monero_miner.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"
#include "monero/monero.h"
#include "monero/monero_job.h"
#include "monero/monero_result.h"
#include "monero/monero_solver.h"

#include "utils/byteswap.h"
#include "utils/hex.h"

#define MONERO_NONCE_POSITION 39

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
};

inline static uint64_t target_to_difficulty(uint64_t t)
{
  return t > 0 ? 0xffffffffffffffff / t : 0;
}

inline static uint64_t t32_to_t64(uint32_t t)
{
  return 0xFFFFFFFFFFFFFFFFULL / (0xFFFFFFFFULL / ((uint64_t)t));
}

inline static uint64_t read_target(const char *str)
{
  size_t len = strlen(str);
  if (len <= 8) {
    uint32_t res = 0;
    hex_to_binary(str, len, (uint8_t *)&res);
    return t32_to_t64(res);
  } else if (len <= 16) {
    uint64_t res = 0;
    hex_to_binary(str, len, (uint8_t *)&res);
    return res;
  } else {
    return 0;
  }
}

void monero_miner_submit(struct monero_solver_solution *solution, void *data)
{
  struct monero_miner *miner = (struct monero_miner *)data;
  assert(miner->event_handler != NULL);
  if (solution->job_id != miner->job_seq_id) {
    log_warn("Stale solution detected!");
    return;
  }
  struct monero_result result = {0, .nonce = 0};
  result.job_id = miner->job_id;
  result.nonce = solution->nonce;
  hex_from_binary(solution->hash, MONERO_OUTPUT_HASH_LEN, result.hash);

  struct miner_event_result_found result_found_evt;
  result_found_evt.event_type = MINER_EVENT_RESULT_FOUND;
  result_found_evt.data = &result;
  miner->event_handler->cb((struct miner_event *)&result_found_evt,
                           miner->event_handler->data);
}

void monero_miner_free(miner_handle *handle)
{
  struct monero_miner *miner = (struct monero_miner *)handle;
  if (miner->job_id != NULL) {
    free((void *)miner->job_id);
  }
  if (miner->solvers_len > 0) {
    for (size_t i = 0; i < miner->solvers_len; ++i) {
      struct monero_solver *solver = miner->solvers[i];
      solver->free(solver);
    }
    free(miner->solvers);
  }
  free((void *)miner);
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
  if (input_hash_len > MONERO_INPUT_HASH_MAX_LEN) {
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
  uint32_t nonce_from = arc4random() % miner->nonce_chunk_size;
  log_debug("Work: %lu. Starting nonce: %x, chunk size: %u", miner->job_seq_id,
            nonce_from, miner->nonce_chunk_size);
  // split nonce into work chunks of equal size
  for (size_t i = 0; i < miner->solvers_len; ++i) {
    uint32_t nonce_to = nonce_from + miner->nonce_chunk_size;
    miner->solvers[i]->work(miner->solvers[i], monero_miner_submit, miner,
                            miner->job_seq_id, input_hash, input_hash_len,
                            miner->target, nonce_from, nonce_to);
    nonce_from = nonce_to;
  }
FREE:
  free(input_hash);
}

miner_handle monero_miner_new(const struct config_miner *cfg)
{
  assert(cfg != NULL);
  struct monero_miner *monero_miner = calloc(1, sizeof(struct monero_miner));
  miner_handle miner = &monero_miner->miner;

  miner->new_job = monero_miner_new_job;
  miner->free = monero_miner_free;

  // TODO: proper initialization based on config
  monero_miner->solvers_len = 4;
  monero_miner->nonce_chunk_size =
      0xffffffff / (uint32_t)(monero_miner->solvers_len + 1);
  monero_miner->solvers = calloc(2, sizeof(struct monero_solver **));
  for (size_t i = 0; i < monero_miner->solvers_len; ++i) {
    monero_miner->solvers[i] = monero_solver_new_cpu();
  }

  return miner;
}
