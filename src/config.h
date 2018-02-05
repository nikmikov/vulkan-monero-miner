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

/** Linked list of miners configurations  */
struct config {
  struct config *next;
  enum currency currency;
  enum stratum_protocol protocol;
  const char *wallet;
  const char *password;
  const char name[32];
  struct config_pool_list pool_list;

  void (*free)(struct config *);
};

struct config *config_from_file(const char *filename);
