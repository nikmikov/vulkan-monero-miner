#include "monero/monero_miner.h"

#include <assert.h>
#include <stdlib.h>

#include "monero/monero_job.h"

struct monero_miner {
  struct miner miner;
};

void monero_miner_free(miner_handle *miner)
{

}

void monero_miner_new_job(miner_handle handle,
                           void *job_data,
                           struct miner_event_handler *event_handler)
{
  struct monero_job *job = (struct monero_job*)job_data;
}

miner_handle monero_miner_new(const struct config_miner *cfg)
{
  assert(cfg != NULL);
  struct monero_miner *monero_miner = calloc(1, sizeof(struct monero_miner));
  miner_handle miner = &monero_miner->miner;

  miner->new_job = monero_miner_new_job;
  miner->free = monero_miner_free;

  return miner;
}
