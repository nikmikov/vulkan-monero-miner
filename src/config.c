#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "logging.h"

#include "cJSON/cJSON.h"


bool stratum_protocol_from_string(const char *str, enum stratum_protocol *out)
{
  if(strcmp(str, "stratum-ethereum") == 0) {
	*out = STRATUM_PROTOCOL_ETHEREUM;
	return true;
  } else if (strcmp(str, "stratum-monero")) {
	*out = STRATUM_PROTOCOL_MONERO;
	return true;
  } else if (strcmp(str, "stratum-zcash")) {
	*out = STRATUM_PROTOCOL_ZCASH;
	return true;
  } else {
	log_error("Unknown stratum protocol: %s", str);
	return false;
  }
}

/** Read file into char* array. Caller is responsible of freeing the allocated memory
 */
char* read_text_file(const char *filename)
{
  FILE *fp = fopen (filename, "r");
  if(!fp) {
	log_error("Unable to open file: %s", filename);
	return NULL;
  }
  fseek(fp , 0L, SEEK_END);

  size_t size_bytes = (size_t)ftell(fp);
  rewind(fp);

  char *buffer = calloc(size_bytes + 1, sizeof(char));
  if(!buffer) {
	log_error("Memory allocation of %d bytes failed", size_bytes);
	fclose(fp);
	return NULL;
  }

  size_t bytes_read = fread(buffer, 1, size_bytes,  fp);
  fclose(fp);
  if(bytes_read != size_bytes) {
	log_error("Error reading form %s, expected read %d bytes, but got %d",
			  filename, size_bytes, bytes_read);
	free(buffer);
	return NULL;
  }
  return buffer;
}

/** Get a string field from json, return NULL if field does not exists
 */
bool read_bool_from_json(const cJSON *json, const char *field, bool *out)
{
  if(!cJSON_HasObjectItem(json, field)) {
	log_error("Field \"%s\" not found", field);
	return false;
  }
  cJSON *item = cJSON_GetObjectItem(json, field);
  if(!cJSON_IsBool(item)) {
	log_error("Field \"%s\" is not a boolean", field);
	return false;
  }

  *out = cJSON_IsTrue(item);
  return true;
}

/** Get a string field from json, return NULL if field does not exists
 */
const char* get_string_from_json(const cJSON *json, const char *field)
{
  if(!cJSON_HasObjectItem(json, field)) {
	log_error("Field \"%s\" not found", field);
	return NULL;
  }
  cJSON *item = cJSON_GetObjectItem(json, field);
  if(!cJSON_IsString(item)) {
	log_error("Field \"%s\" is not a string", field);
	return NULL;
  }

  return item->valuestring;
}

/** Get a string field from json, return NULL if field does not exists
 */
const cJSON* get_array_from_json(const cJSON *json, const char *field)
{
  if(!cJSON_HasObjectItem(json, field)) {
	log_error("Field \"%s\" not found", field);
	return NULL;
  }
  cJSON *item = cJSON_GetObjectItem(json, field);
  if(!cJSON_IsArray(item)) {
	log_error("Field \"%s\" is not an array", field);
	return NULL;
  }

  return item;
}

/** Copy a string field from json into newly allocated string and return pointer to the
 *  allocated string or NULL if fields does not exist
 */
char* copy_string_from_json(const cJSON *json, const char *field)
{
  const char *str = get_string_from_json(json, field);
  if(str == NULL)  {
	return NULL;
  }
  size_t sz = strlen(str);
  char *str_copy = calloc(sz + 1, sizeof(char));
  memcpy(str_copy, str, sz);
  return str_copy;
}

/** Read "currency" field from json
 */
bool read_currency(const cJSON *json, enum currency *currency_ptr)
{
  const char *currency_str = get_string_from_json(json, "currency");
  if(currency_str == NULL)  {
	return false;
  }
  return currency_from_name(currency_str, currency_ptr);
}

/** Read "protocol" field from json
 */
bool read_protocol(const cJSON *json, enum stratum_protocol *protocol_ptr)
{
  const char *protocol_str = get_string_from_json(json, "protocol");
  if(protocol_str == NULL)  {
	return false;
  }
  return stratum_protocol_from_string(protocol_str, protocol_ptr);
}

/** Read "wallet" field from json
 */
bool read_wallet(const cJSON *json, const char **wallet_address_ptr)
{
  assert(*wallet_address_ptr == NULL);
  char *wallet_str = copy_string_from_json(json, "wallet");
  if(wallet_str == NULL)  {
	return false;
  }
  *wallet_address_ptr = wallet_str;
  return true;
}

/** Read pool config from json
 */
bool read_pool(const cJSON *json, struct config_pool* pool)
{
  assert(pool->host == NULL);
  assert(pool->port == NULL);
  if(!read_bool_from_json(json, "use_tls", &pool->use_tls)){
	return false;
  }

  char* host = copy_string_from_json(json, "host");
  if(host == NULL)  {
	return false;
  }
  char* port = copy_string_from_json(json, "port");
  if (port == NULL) {
	free(host);
	return false;
  }
  pool->host = host;
  pool->port = port;
  return true;
}

/** Read pool list from json
 */
bool read_pool_list(const cJSON *json, struct config_pool_list* pool_list)
{
  assert(pool_list->size == 0);
  const cJSON* pool_list_json = get_array_from_json(json, "pool_list");
  if(pool_list_json == NULL) {
	return false;
  }
  int size = cJSON_GetArraySize(pool_list_json);
  if (size <= 0) {
	log_error("Empty pool_list");
	return false;
  }
  pool_list->pools = calloc(size, sizeof(struct config_pool));
  for (int i = 0; i < size; ++i, ++pool_list->size) {
	log_debug("Config parse: parsing poll entry #%d", i);
	cJSON *pool_json = cJSON_GetArrayItem(pool_list_json, i);
	assert(pool_json != NULL);
	if (!read_pool(pool_json, &pool_list->pools[i])) {
	  return false;
	}
  }
  return true;
}

/** TODO: comment here
 */
bool read_miner(const cJSON *json, struct config_miner *cfg)
{
  if(!cJSON_IsObject(json)) {
	log_error("Config parse: \"miners\" list: Expected an object");
	return false;
  }

  return read_currency(json, &cfg->currency)
	&& read_protocol(json, &cfg->protocol)
	&& read_wallet(json, &cfg->wallet)
	&& read_pool_list(json, &cfg->pool_list);
}

/** TODO: comment here
 */
bool config_from_json(const cJSON *json, struct config *cfg)
{
  assert(cfg->size == 0);

  if(!cJSON_IsArray(json)) {
	log_error("Config parse: Expected array of miners");
	return false;
  }

  int size = cJSON_GetArraySize(json);
  if (size <= 0) {
	log_error("Config parse: Empty config");
	return false;
  }

  log_debug("Config parse: entries found: %d", size);
  cfg->miners = calloc(size, sizeof(struct config_miner));
  for (int i = 0; i < size; ++i, ++cfg->size) {
	log_debug("Config parse: parsing miner entry #%d", i);
	cJSON *miner_json = cJSON_GetArrayItem(json, i);
	assert(miner_json != NULL);
	struct config_miner *miner = &cfg->miners[i];
	if (!read_miner(miner_json, miner)) {
	  return false;
	} else {
#ifndef NDEBUG
	  log_debug("- miner: #%d:", i);
	  log_debug("    protocol: %d", miner->protocol);
	  log_debug("    currency: %s", currency_get_info(miner->currency)->name);
	  log_debug("    wallet: %s", miner->wallet);
	  log_debug("    pool(%d):", miner->pool_list.size);
	  for (size_t j = 0; j < miner->pool_list.size; ++j) {
		const struct config_pool *pool = &miner->pool_list.pools[j];
		log_debug("      - host: %s", pool->host);
		log_debug("        port: %s", pool->port);
		log_debug("        tls: %s", pool->use_tls ? "true" : "false");
	  }
#endif
	}
  }

  return true;
}


/** Read config from null terminated string
 */
bool config_from_string(const char *json_str, struct config **cfg_ptr)
{
  log_debug("Parsing config file: %s", json_str);
  bool success = false;
  const char *parse_end;
  cJSON *json_root = cJSON_ParseWithOpts(json_str, &parse_end, true);

  if (json_root != NULL) {
	log_debug("Successfully parsed config json");
	struct config *cfg = calloc(1, sizeof(struct config));

	if (config_from_json(json_root, cfg)) {
	  *cfg_ptr = cfg;
	  success = true;
	} else {
	  config_free(cfg);
	}
  } else {
	// error
    log_error("Json parse error: %s", cJSON_GetErrorPtr());
  }

  cJSON_Delete(json_root);
  return success;
}

/** Read config from file
 */
bool config_from_file(const char *filename, struct config **cfg_ptr) {
  log_debug("Reading config file: %s", filename);
  assert(*cfg_ptr == NULL);
  char *json_str = read_text_file(filename);
  if(!json_str){
	return false;
  }

  bool success = config_from_string(json_str, cfg_ptr);
  free(json_str);
  return success;
}

void config_pool_free(struct config_pool *pool)
{
  assert(pool != NULL);
  free((void*)pool->host);
  free((void*)pool->port);
}

void config_miner_free(struct config_miner *miner)
{
  assert(miner != NULL);
  if(miner->wallet) {
	free((void*)miner->wallet);
  }
  if (miner->pool_list.pools != NULL) {
	for(size_t i = 0; i < miner->pool_list.size; ++i) {
	  config_pool_free(&miner->pool_list.pools[i]);
	}

	free(miner->pool_list.pools);
  }
}

/** Free memory
 */
void config_free(struct config *cfg) {
  assert(cfg != NULL);
  if(cfg->size > 0) {
	for(size_t i = 0; i < cfg->size; ++i) {
	  config_miner_free(&cfg->miners[i]);
	}
	free(cfg->miners);
  }
  free(cfg);
}
