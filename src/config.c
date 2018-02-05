#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"

#include "cJSON/cJSON.h"
#include "monero/monero_config.h"
#include "utils/json.h"

bool stratum_protocol_from_string(const char *str, enum stratum_protocol *out)
{
  if (strcmp(str, "stratum-ethereum") == 0) {
    *out = STRATUM_PROTOCOL_ETHEREUM;
    return true;
  } else if (strcmp(str, "stratum-monero") == 0) {
    *out = STRATUM_PROTOCOL_MONERO;
    return true;
  } else if (strcmp(str, "stratum-zcash") == 0) {
    *out = STRATUM_PROTOCOL_ZCASH;
    return true;
  } else {
    log_error("Unknown stratum protocol: %s", str);
    return false;
  }
}

/** Read file into char* array.
 *  Caller is responsible of freeing the allocated memory */
char *read_text_file(const char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp) {
    log_error("Unable to open file: %s", filename);
    return NULL;
  }
  fseek(fp, 0L, SEEK_END);

  size_t size_bytes = (size_t)ftell(fp);
  rewind(fp);

  char *buffer = calloc(size_bytes + 1, sizeof(char));
  if (!buffer) {
    log_error("Memory allocation of %d bytes failed", size_bytes);
    fclose(fp);
    return NULL;
  }

  size_t bytes_read = fread(buffer, 1, size_bytes, fp);
  fclose(fp);
  if (bytes_read != size_bytes) {
    log_error("Error reading form %s, expected read %d bytes, but got %d",
              filename, size_bytes, bytes_read);
    free(buffer);
    return NULL;
  }
  return buffer;
}

/** Read "currency" field from json */
bool read_currency(const cJSON *json, enum currency *currency_ptr)
{
  const char *currency_str = json_get_string(json, "currency");
  if (currency_str == NULL) {
    return false;
  }
  return currency_from_name(currency_str, currency_ptr);
}

/** Read "protocol" field from json */
bool read_protocol(const cJSON *json, enum stratum_protocol *protocol_ptr)
{
  const char *protocol_str = json_get_string(json, "protocol");
  if (protocol_str == NULL) {
    return false;
  }
  return stratum_protocol_from_string(protocol_str, protocol_ptr);
}

/** Read "wallet" field from json */
bool read_wallet(const cJSON *json, const char **wallet_address_ptr)
{
  assert(*wallet_address_ptr == NULL);
  const char *wallet_str = json_get_string(json, "wallet");
  if (wallet_str == NULL) {
    return false;
  }
  *wallet_address_ptr = strdup(wallet_str);
  return true;
}

/** Read "password" field from json. */
bool read_password(const cJSON *json, const char **password_address_ptr)
{
  assert(*password_address_ptr == NULL);
  const char *password_str = json_get_string(json, "password");
  if (password_str == NULL) {
    *password_address_ptr = strdup("");
    return true;
  }
  *password_address_ptr = strdup(password_str);
  return true;
}

/** Read pool config from json */
bool read_pool(const cJSON *json, struct config_pool *pool)
{
  assert(pool->host == NULL);
  assert(pool->port == NULL);
  if (!json_get_bool(json, "use_tls", &pool->use_tls)) {
    return false;
  }

  const char *host = json_get_string(json, "host");
  if (host == NULL) {
    return false;
  }
  const char *port = json_get_string(json, "port");
  if (port == NULL) {
    return false;
  }
  pool->host = strdup(host);
  pool->port = strdup(port);
  return true;
}

/** Read pool list from json */
bool read_pool_list(const cJSON *json, struct config_pool_list *pool_list)
{
  assert(pool_list->size == 0);
  const cJSON *pool_list_json = json_get_array(json, "pool_list");
  if (pool_list_json == NULL) {
    return false;
  }
  int size = cJSON_GetArraySize(pool_list_json);
  if (size <= 0) {
    log_error("Empty pool_list");
    return false;
  }
  pool_list->pools = calloc(size, sizeof(struct config_pool));
  for (int i = 0; i < size; ++i, ++pool_list->size) {
    log_debug("Config parse: parsing pool entry #%d", i);
    cJSON *pool_json = cJSON_GetArrayItem(pool_list_json, i);
    assert(pool_json != NULL);
    if (!read_pool(pool_json, &pool_list->pools[i])) {
      return false;
    }
  }
  return true;
}

struct config *config_from_json(const cJSON *json)
{
  assert(json != NULL);

  if (!cJSON_IsObject(json)) {
    log_error("Config parse: Miner: Expected object");
    return NULL;
  }

  enum currency currency;
  if (!read_currency(json, &currency)) {
    return NULL;
  }

  struct config *result = NULL;
  switch (currency) {
  case CURRENCY_XMR:
    result = monero_config_from_json(json);
    break;
  case CURRENCY_ETH:
    log_error("Ethereum is not supported yet.");
    break;
  case CURRENCY_ZEC:
    log_error("ZCash is not supported yet.");
    break;
  }
  if (result == NULL) {
    return NULL;
  }
  assert(result->next == NULL);

  // read generic fields
  bool succ = read_protocol(json, &result->protocol) &&
              read_wallet(json, &result->wallet) &&
              read_password(json, &result->password) &&
              read_pool_list(json, &result->pool_list);

  if (!succ) {
    result->free(result);
    result = NULL;
  }
  return result;
}

/** Read config from null terminated string */
struct config *config_from_string(const char *json_str)
{
  log_debug("Parsing config file: %s", json_str);
  const char *parse_end;
  cJSON *json_root = cJSON_ParseWithOpts(json_str, &parse_end, true);
  if (json_root == NULL) {
    log_error("Json parse error: %s", cJSON_GetErrorPtr());
    return NULL;
  }
  log_debug("Successfully parsed config json");

  struct config *result = NULL;

  if (!cJSON_IsArray(json_root)) {
    log_error("Config parse: Expected array of miners");
    cJSON_Delete(json_root);
    return NULL;
  }

  int size = cJSON_GetArraySize(json_root);
  if (size <= 0) {
    log_error("Config parse: Empty config");
    cJSON_Delete(json_root);
    return NULL;
  }
  log_debug("Config parse: entries found: %d", size);
  for (int i = size - 1; i >= 0; --i) {
    // parse config
    log_debug("Config parse: parsing miner entry #%d", i);
    const cJSON *miner_json = cJSON_GetArrayItem(json_root, i);
    struct config *cfg = config_from_json(miner_json);
    if (cfg == NULL) {
      if (result != NULL) {
        result->free(result);
        result = NULL;
      }
      break;
    }
    cfg->next = result;
    result = cfg;
  }

  cJSON_Delete(json_root);
  return result;
}

struct config *config_from_file(const char *filename)
{
  log_debug("Reading config file: %s", filename);
  char *json_str = read_text_file(filename);
  if (!json_str) {
    return false;
  }

  struct config *res = config_from_string(json_str);
  free(json_str);
  return res;
}
