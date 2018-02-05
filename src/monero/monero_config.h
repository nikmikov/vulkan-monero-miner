#pragma once

#include "cJSON/cJSON.h"

#include "config.h"

struct monero_config {
  struct config config;
};

struct config *monero_config_from_json(const cJSON *json);
