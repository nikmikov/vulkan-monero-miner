/* connection.h -- low level connection interface to mining pool
 *
 */
#pragma once

#include <stdbool.h>

#include <uv.h>

#include "config.h"

enum connection_event_type {
  CONNECTION_EVENT_NOEVENT,
  CONNECTION_EVENT_CONNECTED,
  CONNECTION_EVENT_DATA
};

struct connection;
struct connection_event_handler;

typedef struct connection *connection_handle;
typedef void (*connection_event_cb)(enum connection_event_type event,
                                    const uv_buf_t *event_data, void *data);

struct connection_event_handler {
  void *data;
  connection_event_cb cb;
};

/** Create and initialize connection */
connection_handle connection_init(const struct config_pool_list *pool_list);

void connection_start(connection_handle, struct connection_event_handler *);

void connection_stop(connection_handle);

bool connection_is_connected(connection_handle);

void connection_write(connection_handle, uv_buf_t data);

void connection_free(connection_handle *);
