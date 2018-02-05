#pragma once

#include "cJSON/cJSON.h"

#include "config.h"

enum monero_config_solver_type {
  MONERO_CONFIG_SOLVER_CPU,
  MONERO_CONFIG_SOLVER_CL,
  MONERO_CONFIG_SOLVER_CUDA
};

struct monero_config_solver {
  struct monero_config_solver *next;
  enum monero_config_solver_type solver_type;
  int affine_to_cpu; /** CPU affinity or -1 for no affinity*/
};

struct monero_config_solver_cpu {
  struct monero_config_solver solver;
};

struct monero_config_solver_cl {
  struct monero_config_solver solver;
};

struct monero_config_solver_cuda {
  struct monero_config_solver solver;
};

struct monero_config {
  struct config config;
  struct monero_config_solver *solvers_list;
};

struct config *monero_config_from_json(const cJSON *json);
