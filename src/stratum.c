#include "stratum.h"

#include <assert.h>

#include "logging.h"
#include "monero/monero_stratum.h"


stratum_handle stratum_new(const struct config_miner *cfg)
{
  switch(cfg->protocol) {
  case STRATUM_PROTOCOL_MONERO:
    return (stratum_handle)monero_stratum_new(cfg->wallet, cfg->password);
  default:
    log_error("Unsupported stratum protocol: %d", cfg->protocol);
    assert(false);
    return NULL;
  }
}

void stratum_free(stratum_handle *stratum)
{
  assert(*stratum != NULL);
  enum stratum_protocol protocol = (*stratum)->protocol;
  switch(protocol) {
  case STRATUM_PROTOCOL_MONERO:
    monero_stratum_free((monero_stratum_handle*)stratum);
    break;
  default:
    log_error("Unsupported stratum protocol: %d", protocol);
    assert(false);
  }
}
