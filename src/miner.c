#include "miner.h"

#include <assert.h>

#include "logging.h"
#include "monero/monero_miner.h"

miner_handle miner_new(const struct config_miner *cfg)
{
  switch(cfg->protocol) {
  case STRATUM_PROTOCOL_MONERO:
    return (miner_handle)monero_miner_new(cfg);
  default:
    log_error("Unsupported stratum protocol: %d", cfg->protocol);
    assert(false);
    return NULL;
  }
}
