#include "monero/monero_config.h"

#include "currency.h"
#include <stdlib.h>

void monero_config_free(struct config *ptr)
{
  struct monero_config *cfg = (struct monero_config *)ptr;
}

struct config *monero_config_from_json(const cJSON *json)
{
  struct monero_config *cfg = calloc(1, sizeof(struct monero_config));
  cfg->config.currency = CURRENCY_XMR;
  cfg->config.free = monero_config_free;
  return &cfg->config;
}
