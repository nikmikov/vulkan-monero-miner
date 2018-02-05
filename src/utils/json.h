#pragma once

#include "cJSON/cJSON.h"
#include "logging.h"

static inline bool json_get_object(const cJSON *json, const char *field,
                                   const cJSON **out)
{
  if (!cJSON_HasObjectItem(json, field)) {
    log_error("Field \"%s\" not found", field);
    return false;
  }
  const cJSON *item = cJSON_GetObjectItem(json, field);
  *out = item;
  return true;
}

/** Read boolean field from json, return false if field does not exists */
static inline bool json_get_bool(const cJSON *json, const char *field,
                                 bool *out)
{
  const cJSON *item = NULL;
  if (!json_get_object(json, field, &item)) {
    return false;
  }
  if (!cJSON_IsBool(item)) {
    log_error("Field \"%s\" is not a boolean", field);
    return false;
  }

  *out = cJSON_IsTrue(item);
  return true;
}

/** Get a string field from json, return NULL if the field does not exists */
static inline const char *json_get_string(const cJSON *json, const char *field)
{
  const cJSON *item = NULL;
  if (!json_get_object(json, field, &item)) {
    return false;
  }
  if (!cJSON_IsString(item)) {
    log_error("Field \"%s\" is not a string", field);
    return NULL;
  }

  return item->valuestring;
}

/** Get array field from json, return NULL if the field does not exists */
static inline const cJSON *json_get_array(const cJSON *json, const char *field)
{
  const cJSON *item = NULL;
  if (!json_get_object(json, field, &item)) {
    return NULL;
  }
  if (!cJSON_IsArray(item)) {
    log_error("Field \"%s\" is not an array", field);
    return NULL;
  }

  return item;
}
