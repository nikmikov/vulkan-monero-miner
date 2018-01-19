#ifndef MONERO_WORKER_H
#define MONERO_WORKER_H

#include "config.h"
#include "worker.h"

worker_handle monero_worker_new(const struct config_miner *cfg);


#endif /** MONERO_WORKER_H */
