#include "foreman.h"

#include <assert.h>
#include <stdlib.h>

#include "logging.h"
#include "connection.h"
#include "miner.h"
#include "stratum.h"


struct foreman {
  const struct config_miner *cfg;
  connection_handle connection;
  struct connection_event_handler connection_event_handler;
  stratum_handle stratum;
  struct stratum_event_handler stratum_event_handler;
  miner_handle miner;
  struct miner_event_handler miner_event_handler;
};

void on_miner_event(const struct miner_event *event, void *data)
{
  struct foreman *foreman = data;
  assert(foreman != NULL);
  assert(event != NULL);
  switch(event->event_type) {
  case MINER_EVENT_RESULT_FOUND:
    log_info("Result found!");
    foreman->stratum->submit(foreman->stratum, ((struct miner_event_result_found*)event)->data);
    break;
  case MINER_EVENT_SEARCH_SPACE_EXHAUSTED:
    log_info("Search space exhausted!");
    break;
  default:
    log_error("Invalid miner event type: %d", event->event_type);
    assert(false);
  }
}

void on_stratum_event(const struct stratum_event *event, void *data)
{
  struct foreman *foreman = data;
  assert(foreman != NULL);
  assert(event != NULL);
  switch(event->event_type) {
  case STRATUM_EVENT_INVALID_REPLY: {
    const char *err = ((const struct stratum_event_invalid_reply*)event)->error;
    log_error("%s: Invalid reply d: %s, error: %s", foreman->cfg->name, err);
    break;
  }
  case STRATUM_EVENT_LOGIN_FAILED: {
    const char *err = ((const struct stratum_event_login_failed*)event)->error;
    log_error("%s: Login failed. Error: %s", foreman->cfg->name, err);
    break;
  }
  case STRATUM_EVENT_LOGIN_SUCCESS: {
    log_debug("%s: Login result event", foreman->cfg->name);
    break;
  }
  case STRATUM_EVENT_NEW_JOB: {
    log_debug("%s: New job event", foreman->cfg->name);
    void *job_data = ((struct stratum_event_new_job*)event)->job_data;
    foreman->miner->new_job(foreman->miner, job_data, &foreman->miner_event_handler);
    break;
  }
  default:
    log_error("Invalid stratum event type: %d", event->event_type);
    assert(false);
  }
}

void on_connection_event(enum connection_event_type event, const uv_buf_t *event_data, void *data)
{
  struct foreman *foreman = data;
  switch(event) {
  case CONNECTION_EVENT_CONNECTED:
    log_debug("%s: connection CONNECTED", foreman->cfg->name);
    assert(event_data == NULL);
    assert(foreman->stratum->login != NULL);
    foreman->stratum->login(foreman->stratum, foreman->connection, &foreman->stratum_event_handler);
    break;
  case CONNECTION_EVENT_DATA:
    log_debug("%s: connection DATA", foreman->cfg->name);
    assert(event_data != NULL);
    assert(foreman->stratum->new_payload != NULL);
    if(event_data->len > 0) {
      foreman->stratum->new_payload(foreman->stratum, event_data);
    }
    break;
  default:
    log_error("Invalid connection event type: %d", event);
    assert(false);
  }
}

foreman_handle foreman_init(const struct config_miner *cfg)
{
  log_debug("Initializing foreman: %s", cfg->name);
  connection_handle pool_connection = connection_init(&cfg->pool_list);
  if (pool_connection == NULL) {
    log_error("Failed initialize connection");
    return NULL;
  }

  stratum_handle stratum = stratum_new(cfg);
  if(stratum == NULL) {
    log_error("Failed initialize stratum");
    connection_free(&pool_connection);
    return NULL;
  }

  miner_handle miner = miner_new(cfg);
  if(miner == NULL) {
    log_error("Failed initialize miner");
    connection_free(&pool_connection);
    stratum_free(&stratum);
    return NULL;
  }

  struct foreman *foreman = calloc(1, sizeof(struct foreman));
  foreman->cfg = cfg;
  foreman->connection = pool_connection;
  foreman->connection_event_handler.data = foreman;
  foreman->connection_event_handler.cb = on_connection_event;

  foreman->stratum_event_handler.data = foreman;
  foreman->stratum_event_handler.cb = on_stratum_event;
  foreman->stratum = stratum;

  foreman->miner_event_handler.data = foreman;
  foreman->miner_event_handler.cb = on_miner_event;
  foreman->miner = miner;

  return foreman;
}

void foreman_start(foreman_handle foreman)
{
  log_debug("Starting foreman: %s", foreman->cfg->name);
  assert(foreman != NULL);
  assert(foreman->connection != NULL);
  connection_start(foreman->connection, &foreman->connection_event_handler);
}

void foreman_stop(foreman_handle foreman)
{
  log_debug("Stopping foreman: %s", foreman->cfg->name);
  assert(foreman != NULL);
  assert(foreman->connection != NULL);
  connection_stop(foreman->connection);
}

void foreman_free(foreman_handle *foreman_ptr)
{
  assert(*foreman_ptr != NULL);
  struct foreman *foreman = *foreman_ptr;
  connection_free(&foreman->connection);
  stratum_free(&foreman->stratum);
  assert(foreman->miner->free != NULL);
  foreman->miner->free(&foreman->miner);
  free(foreman);
  *foreman_ptr = NULL;
}
