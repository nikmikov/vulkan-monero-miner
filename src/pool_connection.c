#include "pool_connection.h"

#include <assert.h>
#include <stdlib.h>

#include <uv.h>

#include "logging.h"

#define INITIAL_RETRY_DELAY_MILLISEC (5 * 1000)
#define RETRY_DELAY_INCREASE_RATE 2


struct connection_data {
  /** const data from config */
  const struct config_pool *config_pool;

  // Index in pool_connection->connection_data. Also represents connection priority
  size_t index;

  // UV structures
  uv_getaddrinfo_t resolver;
  uv_connect_t connect_req;
  uv_tcp_t socket;

  // resolved address
  struct addrinfo *addrinfo;

  // time delay in milliseconds until next retry
  uint64_t retry_delay;

  // keep track of failed connection attempts
  size_t failed_attempts;
};

/** List of connections ordered by priority (highest first) */
struct pool_connection {
  size_t size;
  struct connection_data *data;
};

/** Hostname resolve functions */
void on_resolve_retry_timer_cb(uv_timer_t* handle);
void on_resolve_failed(struct connection_data *conn, int status);
void on_resolved_cb(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res);
void resolve_async(struct connection_data* conn);

/** */
void on_resolve_retry_timer_cb(uv_timer_t* handle)
{
  resolve_async(handle->data);
}

/** */
void on_resolve_failed(struct connection_data *conn, int status)
{
  uv_loop_t* loop = uv_default_loop();

  const char* host = conn->config_pool->host;
  const char* port = conn->config_pool->port;

  log_error("Error when resolving %s:%s: %s", host, port, uv_err_name(status));
  conn->failed_attempts++;
  // set retry delay
  uv_timer_t timer_req;
  timer_req.data = conn;
  uv_timer_init(loop, &timer_req);
  uv_timer_start(&timer_req,  on_resolve_retry_timer_cb, conn->retry_delay, 0);
  conn->retry_delay *= RETRY_DELAY_INCREASE_RATE;
}

/** callback on resolved */
void on_resolved_cb(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res)
{
  struct connection_data *conn = resolver->data;
  assert(conn->addrinfo == NULL && "Expect address is not resolved");

  if (status < 0) {
    on_resolve_failed(conn, status);
    return;
  }
  // success
  conn->addrinfo = res;
  // reset failed attempts counter and delay
  conn->failed_attempts = 0;
  conn->retry_delay = INITIAL_RETRY_DELAY_MILLISEC;

  /*
  int i0 = -conn->index;
  for (struct connection_data *c = &conn[i0] ; c <= conn; ++c) {
    if (ready_to_connect(c)) {
      connect_async(c);
      return;
    }
  }
  */


  //char addr[17] = {'\0'};
  //uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);

  //log_info("Address resolved: %s", addr);

}


/** Resolve hostname asynchronously */
void resolve_async(struct connection_data* conn)
{
  uv_loop_t* loop = uv_default_loop();

  struct addrinfo hints;
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  const char* host = conn->config_pool->host;
  const char* port = conn->config_pool->port;
  log_info("Resolving: %s:%s. Attempt #%d", host, port, conn->failed_attempts);

  int err_code = uv_getaddrinfo(loop, &conn->resolver, on_resolved_cb, host, port, &hints);
  if (err_code) {
    on_resolve_failed(conn, err_code);
    return;
  }
}

/*
void on_connect_cb(uv_connect_t* req, int status)
{
  struct connection_data *conn = req->data;
  if (status < 0) {
    log_error("Connection callback error %s", uv_err_name(status));
    conn->failed_attempts++;
    return;
  }
  conn->failed_attempts = 0; // reset failed attempts counter
}


void connect_async(struct connection_data *conn)
{
  uv_loop_t* loop = uv_default_loop();
  const struct sockaddr *sockaddr = conn->addrinfo->ai_addr;

  uv_tcp_init(loop, &conn->socket);
  int err_code = uv_tcp_connect(&conn->connect_req, &conn->socket, sockaddr, on_connect_cb);
  if(err_code) {
    log_error("Error when establishing TCP connection: %s", uv_err_name(err_code));
    conn->failed_attempts++;
    // set retry delay
  }
}

*/

/** Initialize data structures */
bool pool_connection_init(const struct config_pool_list* cfg,
                          pool_connection_handle* handle_ptr)
{
  assert(handle_ptr == NULL && "Can not initialize non-null handle");

  if(cfg->size == 0) {
    log_error("No hosts configured!");
    return false;
  }

  pool_connection_handle handle = malloc(sizeof(struct pool_connection));
  handle->size = cfg->size;

  handle->data = malloc(sizeof(struct connection_data) * cfg->size);
  struct connection_data* conn = handle->data;
  for (size_t i = 0; i < handle->size; ++i, ++conn) {
    conn->config_pool = &cfg->pools[i];  // assume config is not freed while app is running
    conn->index = i;
    conn->failed_attempts = 0;
    conn->retry_delay = INITIAL_RETRY_DELAY_MILLISEC;
    conn->addrinfo = NULL;
    conn->resolver.data = conn;
    conn->connect_req.data = conn;
  }

  *handle_ptr = handle;

  return true;
}

void pool_connection_connect(pool_connection_handle handle)
{
  struct connection_data* conn = handle->data;
  for (size_t i = 0; i < handle->size; ++i, ++conn) {
    resolve_async(conn);
  }
}

bool pool_connection_shutdown(pool_connection_handle handle)
{
  return false;
}

void pool_connection_free(pool_connection_handle handle)
{
  assert(handle != NULL);
  struct connection_data* conn = handle->data;
  for (size_t i = 0; i < handle->size; ++i, ++conn) {
    uv_freeaddrinfo(conn->addrinfo);
  }
  free(handle->data);
  free(handle);
}
