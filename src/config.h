/* config.h -- Configuration file handling
 *
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "currency.h"

enum stratum_protocol {
  STRATUM_PROTOCOL_ETHEREUM,
  STRATUM_PROTOCOL_MONERO,
  STRATUM_PROTOCOL_ZCASH
};

enum backend { BACKEND_CPU, BACKEND_CUDA, BACKEND_OPENCL };

/** Root config */
struct config {
  size_t size;
  struct config_miner *miners;
};

/** Single host */
struct config_pool {
  const char *host;
  const char *port;
  bool use_tls;
};

/** Multiple hosts for failover */
struct config_pool_list {
  size_t size;
  struct config_pool *pools;
};

/** Single miner config */
struct config_miner {
  enum currency currency;
  enum stratum_protocol protocol;
  const char *wallet;
  const char *password;
  const char name[32];
  struct config_pool_list pool_list;
};

bool stratum_protocol_from_string(const char *str, enum stratum_protocol *out);

bool config_from_file(const char *filename, struct config **cfg);

void config_free(struct config *cfg);
