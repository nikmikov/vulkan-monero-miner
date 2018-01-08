#include "miner.h"

#include <assert.h>
#include <stdlib.h>

#include "logging.h"
#include "connection.h"
#include "stratum.h"

struct miner {
  const struct config_miner *cfg;
  connection_handle connection;
  struct connection_event_handler connection_event_handler;
  stratum_handle stratum;
  struct stratum_event_handler stratum_event_handler;
};

void on_stratum_event(enum stratum_event_type event,
                      stratum_payload_handle payload,
                      void *data)
{
  struct miner *miner = data;
  assert(miner != NULL);
  assert(payload != NULL);
  switch(event) {
  case STRATUM_EVENT_LOGIN_RESULT:
    log_debug("Login result event: %s", miner->cfg->name);
    break;
  case STRATUM_EVENT_NEW_JOB:
    log_debug("New job event: %s", miner->cfg->name);
    break;
  default:
    log_error("Invalid stratum event type: %d", event);
    assert(false);
  }
}

void on_connection_event(enum connection_event_type event, const uv_buf_t *event_data, void *data)
{
  struct miner *miner = data;
  switch(event) {
  case CONNECTION_EVENT_CONNECTED:
    log_debug("%s: connection CONNECTED", miner->cfg->name);
    assert(event_data == NULL);
    assert(miner->stratum->login != NULL);
    miner->stratum->login(miner->stratum, miner->connection, &miner->stratum_event_handler);
    break;
  case CONNECTION_EVENT_DATA:
    log_debug("%s: connection DATA", miner->cfg->name);
    miner->stratum->new_payload(miner->stratum, event_data);
    assert(event_data != NULL);
  default:
    log_error("Invalid connection event type: %d", event);
    assert(false);
  }
}

miner_handle miner_init(const struct config_miner *cfg)
{
  log_debug("Initializing miner: %s", cfg->name);
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

  struct miner *miner = calloc(1, sizeof(struct miner));
  miner->cfg = cfg;
  miner->connection = pool_connection;
  miner->connection_event_handler.data = miner;
  miner->connection_event_handler.cb = on_connection_event;

  miner->stratum_event_handler.data = miner;
  miner->stratum_event_handler.cb = on_stratum_event;
  miner->stratum = stratum;

  return miner;
}

void miner_start(miner_handle miner)
{
  log_debug("Starting miner: %s", miner->cfg->name);
  assert(miner != NULL);
  assert(miner->connection != NULL);
  connection_start(miner->connection, &miner->connection_event_handler);
}

void miner_stop(miner_handle miner)
{
  log_debug("Stopping miner: %s", miner->cfg->name);
  assert(miner != NULL);
  assert(miner->connection != NULL);
  connection_stop(miner->connection);
}

void miner_free(miner_handle *miner_ptr)
{
  assert(*miner_ptr != NULL);
  connection_free(&(*miner_ptr)->connection);
  stratum_free(&(*miner_ptr)->stratum);
  free(*miner_ptr);
  *miner_ptr = NULL;
}
