#pragma once

#include "config.h"
#include "miner.h"

miner_handle monero_miner_new(const struct config_miner *cfg);
