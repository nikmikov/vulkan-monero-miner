#pragma once

#include "miner.h"
#include "monero/monero_config.h"

miner_handle monero_miner_new(const struct monero_config *cfg);
