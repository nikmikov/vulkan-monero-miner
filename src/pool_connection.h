/* pool_handler.h -- abstracts low level connection interface to mining pool
 *
 */
#ifndef POOL_CONNECTION_H
#define POOL_CONNECTION_H

#include <stdbool.h>

#include "config.h"

struct pool_connection;
typedef struct pool_connection* pool_connection_handle;

/** Initialize connection */
bool pool_connection_init(const struct config_pool_list* pool_list,
                          pool_connection_handle* handle);

void pool_connection_connect(pool_connection_handle);

bool pool_connection_is_connected(pool_connection_handle);

void pool_connection_free(pool_connection_handle);


#endif /** POOL_CONNECTION_H */
