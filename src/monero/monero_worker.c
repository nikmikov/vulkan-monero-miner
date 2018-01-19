#include "monero/monero_worker.h"

#include <assert.h>
#include <stdlib.h>

struct monero_worker {
  struct worker worker;
};

void monero_worker_free(worker_handle *worker)
{

}

void monero_worker_new_job(worker_handle handle,
                           void *job_data,
                           struct worker_event_handler *event_handler)
{

}

worker_handle monero_worker_new(const struct config_miner *cfg)
{
  assert(cfg != NULL);
  struct monero_worker *monero_worker = calloc(1, sizeof(struct monero_worker));
  worker_handle worker = &monero_worker->worker;

  worker->new_job = monero_worker_new_job;
  worker->free = monero_worker_free;

  return worker;
}
