#include "monero/monero_config.h"

#include "currency.h"
#include "utils/json.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void monero_config_solver_free(struct monero_config_solver *s)
{
  switch (s->solver_type) {
  case MONERO_CONFIG_SOLVER_CPU:
    break;
  case MONERO_CONFIG_SOLVER_CL:
    break;
  case MONERO_CONFIG_SOLVER_VK:
    break;
  }
}

void monero_config_solver_list_free(struct monero_config_solver **solver_ptr)
{
  struct monero_config_solver *s = *solver_ptr;
  while (s != NULL) {
    struct monero_config_solver *n = s->next;
    monero_config_solver_free(s);
    s = n;
  }
  *solver_ptr = NULL;
}

struct monero_config_solver *
monero_config_solver_cpu_from_json(const cJSON *json)
{
  assert(json != NULL);
  if (!cJSON_IsObject(json)) {
    log_error("CPU solver config is not a JSON object, %s", cJSON_Print(json));
    return NULL;
  }
  // read affinity
  const cJSON *json_affinity;
  if (!json_get_object(json, "affine_to_cpu", &json_affinity)) {
    return NULL;
  }
  int affinity = -1;
  if (cJSON_IsNumber(json_affinity)) {
    affinity = json_affinity->valueint;
  } else if (!cJSON_IsFalse(json_affinity)) {
    log_error("CPU affinity must be a number or `false`");
    return NULL;
  }

  struct monero_config_solver_cpu *res =
      calloc(1, sizeof(struct monero_config_solver_cpu));
  res->solver.solver_type = MONERO_CONFIG_SOLVER_CPU;
  res->solver.affine_to_cpu = affinity;
  return &res->solver;
}

struct monero_config_solver *
monero_config_solver_vk_from_json(const cJSON *json)
{
  assert(json != NULL);
  if (!cJSON_IsObject(json)) {
    log_error("VK solver config is not a JSON object, %s", cJSON_Print(json));
    return NULL;
  }

  // read affinity
  const cJSON *json_affinity;
  if (!json_get_object(json, "affine_to_cpu", &json_affinity)) {
    return NULL;
  }
  int affinity = -1;
  if (cJSON_IsNumber(json_affinity)) {
    affinity = json_affinity->valueint;
  } else if (!cJSON_IsFalse(json_affinity)) {
    log_error("CPU affinity must be a number or `false`");
    return NULL;
  }

  int device_id = -1, parallelism = -1;

  if (!json_get_uint(json, "device", &device_id)) {
    return NULL;
  }

  if (!json_get_uint(json, "parallelism", &parallelism)) {
    return NULL;
  }

  struct monero_config_solver_vk *res =
      calloc(1, sizeof(struct monero_config_solver_vk));
  res->solver.solver_type = MONERO_CONFIG_SOLVER_VK;
  res->solver.affine_to_cpu = affinity;
  res->parallelism = parallelism;
  res->device_id = device_id;
  return &res->solver;
}

struct monero_config_solver *
monero_config_solver_cl_from_json(const cJSON *json)
{
  assert(json != NULL);
  if (!cJSON_IsObject(json)) {
    log_error("CL solver config is not a JSON object, %s", cJSON_Print(json));
    return NULL;
  }
  // read affinity
  const cJSON *json_affinity;
  if (!json_get_object(json, "affine_to_cpu", &json_affinity)) {
    return NULL;
  }
  int affinity = -1;
  if (cJSON_IsNumber(json_affinity)) {
    affinity = json_affinity->valueint;
  } else if (!cJSON_IsFalse(json_affinity)) {
    log_error("CPU affinity must be a number or `false`");
    return NULL;
  }

  int platform_id = -1, device_id = -1, intensity = -1, worksize = -1;
  if (!json_get_uint(json, "platform", &platform_id)) {
    return NULL;
  }

  if (!json_get_uint(json, "device", &device_id)) {
    return NULL;
  }

  if (!json_get_uint(json, "intensity", &intensity)) {
    return NULL;
  }

  if (!json_get_uint(json, "worksize", &worksize)) {
    return NULL;
  }

  struct monero_config_solver_cl *res =
      calloc(1, sizeof(struct monero_config_solver_cl));
  res->solver.solver_type = MONERO_CONFIG_SOLVER_CL;
  res->solver.affine_to_cpu = affinity;
  res->intensity = intensity;
  res->worksize = worksize;
  res->platform_id = platform_id;
  res->device_id = device_id;
  return &res->solver;
}

struct monero_config_solver *monero_config_solver_from_json(const cJSON *json)
{
  assert(json != NULL);
  if (!cJSON_IsObject(json)) {
    log_error("Solver config is not an object");
    return NULL;
  }
  if (cJSON_GetArraySize(json) != 1) {
    log_error("Error when parsing solver config: expecting one of \"cpu\", "
              "\"cl\", \"cuda\"");
    return NULL;
  }
  const cJSON *json_solver = cJSON_GetArrayItem(json, 0);

  const char *type_str = json_solver->string;
  if (strcmp(type_str, "cpu") == 0) {
    return monero_config_solver_cpu_from_json(json_solver);
  } else if (strcmp(type_str, "cl") == 0) {
    return monero_config_solver_cl_from_json(json_solver);
  } else if (strcmp(type_str, "vk") == 0) {
    return monero_config_solver_vk_from_json(json_solver);
  } else {
    log_error("Unknown solver type: %s", type_str);
  }
  return NULL;
}

void monero_config_free(struct config *ptr)
{
  struct monero_config *cfg = (struct monero_config *)ptr;
  if (cfg->config.wallet != NULL) {
    free((void *)cfg->config.wallet);
  }
  if (cfg->config.password != NULL) {
    free((void *)cfg->config.password);
  }
}

struct config *monero_config_from_json(const cJSON *json)
{
  assert(json != NULL);

  // read solvers
  const cJSON *json_solvers_array = json_get_array(json, "solvers");
  if (json_solvers_array == NULL) {
    log_error("No solvers configured");
    return NULL;
  }
  int size = cJSON_GetArraySize(json_solvers_array);
  log_debug("Solvers found: %d", size);
  struct monero_config_solver *solvers_list = NULL;
  for (int i = size - 1; i >= 0; --i) {
    const cJSON *json_solver = cJSON_GetArrayItem(json_solvers_array, i);
    struct monero_config_solver *solver =
        monero_config_solver_from_json(json_solver);
    if (solver == NULL) {
      monero_config_solver_list_free(&solvers_list);
      return NULL;
    }
    solver->next = solvers_list;
    solvers_list = solver;
  }

  struct monero_config *cfg = calloc(1, sizeof(struct monero_config));
  cfg->config.currency = CURRENCY_XMR;
  cfg->config.free = monero_config_free;
  cfg->solvers_list = solvers_list;

  return &cfg->config;
}
