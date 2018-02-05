#include "miner.h"

#include <assert.h>

#include "logging.h"
#include "monero/monero_miner.h"

miner_handle miner_new(const struct config *cfg)
{
  switch (cfg->protocol) {
  case STRATUM_PROTOCOL_MONERO:
    return (miner_handle)monero_miner_new((struct monero_config *)cfg);
  default:
    log_error("Unsupported stratum protocol: %d", cfg->protocol);
    assert(false);
    return NULL;
  }
}
