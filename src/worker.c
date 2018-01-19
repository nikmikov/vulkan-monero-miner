#include "worker.h"

#include <assert.h>

#include "logging.h"
#include "monero/monero_worker.h"

worker_handle worker_new(const struct config_miner *cfg)
{
  switch(cfg->protocol) {
  case STRATUM_PROTOCOL_MONERO:
    return (worker_handle)monero_worker_new(cfg);
  default:
    log_error("Unsupported stratum protocol: %d", cfg->protocol);
    assert(false);
    return NULL;
  }
}
