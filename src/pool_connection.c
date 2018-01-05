#include "pool_connection.h"

#include <assert.h>
#include <stdlib.h>

#include <uv.h>

#include "logging.h"

#define INITIAL_RETRY_DELAY_MILLISEC (1 * 1000)
#define MAX_RETRY_DELAY_MILLISEC 5 * 60 * 1000 // 5 minutes
#define RETRY_DELAY_INCREASE_RATE 2.7182818

struct connection {
  struct pool_connection *pool;
  /** const data from config */
  const struct config_pool *config_pool;

  ///// DNS RESOLVE
  // DNS resolve handle
  uv_getaddrinfo_t resolver;

  // resolved address structure
  struct addrinfo *addrinfo;

  // pointer to current addrinfo, will move to addrinfo->next with each unsuccessfull connect
  struct addrinfo *addrinfo_current;

  // time delay in milliseconds until next retry
  uint64_t resolve_retry_delay;

  // retry timer handle
  uv_timer_t resolve_retry_timer;

  // keep track of failed resolve attempts
  size_t resolve_failed_attempts;

  ///// CONNECT
  // connection request handle
  uv_connect_t connect_req;

  // socket handle
  uv_tcp_t socket;
  bool is_connected;

};

struct pool_connection {
  size_t size;
  struct connection *connections;
  struct connection *connections_end;

  struct connection *active; /** Current active connection or NULL  */
  uv_timer_t connect_timer;
};

/** reset connection to it's initial state, freeing resources when necessary and closing handles */
void connection_reset(struct connection *conn);
/** return true if connection address is resolved */
bool connection_is_resolved(const struct connection *conn);
/** return true if connection is open  */
bool connection_is_connected(const struct connection *conn);
/** return true if connection is open  */
int connection_get_id(const struct connection *conn);
/** asynchronous DNS resolution */
void connection_resolve_async(struct connection *conn);
/** if getaddrinfo failed, retry after a delay */
void connection_resolve_failed(struct connection *conn, int status);
/** if getaddrinfo successfull, initiate TCP connection */
void connection_resolve_success(struct connection *conn, struct addrinfo *res);
/** try to establish TCP connection asynchronously */
void connection_connect_async(struct connection *conn);
/** unable to establish TCP connection */
void connection_connect_failed(struct connection *conn, int status);
/** connection successful */
void connection_connect_success(struct connection *conn);

/** print connection status to debug log */
void connection_log_status(const struct connection *conn);

/** switch active connection */
void pool_connection_switch_active(struct pool_connection *pool)
{
  for (struct connection *conn = pool->connections; conn != pool->connections_end; ++conn) {
    if (connection_is_connected(conn)) {
      if(conn != pool->active) {
        if(pool->active == NULL) {
          log_debug("Setting active connection to: %d", connection_get_id(conn));
        } else {
          log_debug("Switching active connection %d => %d",
                    connection_get_id(pool->active),
                    connection_get_id(conn));
        }
        pool->active = conn;
      }
      return;
    }
  }
}

/* ============    Utility Functions    ============== */
const char* ip_addr_to_str(const struct addrinfo *rp, char* buf)
{
  switch(rp->ai_family) {
  case AF_INET:
    uv_ip4_name((struct sockaddr_in*) rp->ai_addr, buf, 16);
    break;
  case AF_INET6:
    uv_ip6_name((struct sockaddr_in6*) rp->ai_addr, buf, 45);
    break;
  }
  return buf;
}

/* ============       Callbacks         ============== */
void on_getaddrinfo(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res)
{
  struct connection *conn = resolver->data;
  assert(conn->addrinfo == NULL && "Expect address is not resolved yet");

  if (status < 0) {
    connection_resolve_failed(conn, status);
  } else {
    connection_resolve_success(conn, res);
  }
}

void on_resolve_retry_timer(uv_timer_t* handle)
{
  connection_resolve_async(handle->data);
}

void on_tcp_connect(uv_connect_t* req, int status)
{
  struct connection *conn = req->data;
  if (status < 0) {
    connection_connect_failed(conn, status);
  } else {
    assert((void*)req->handle == (void*)&conn->socket);
    connection_connect_success(conn);
  }
}

/* ============  Connection Functions   ============== */
void connection_reset(struct connection *conn)
{
  uv_freeaddrinfo(conn->addrinfo);
  conn->addrinfo = conn->addrinfo_current = NULL;

  conn->resolve_retry_delay = INITIAL_RETRY_DELAY_MILLISEC;
  conn->resolve_failed_attempts = 0;
  conn->is_connected = false;
}

bool connection_is_resolved(const struct connection *conn)
{
  return conn->addrinfo != NULL;
}

bool connection_is_connected(const struct connection *conn)
{
  const uv_stream_t *s = (const uv_stream_t*)&conn->socket;
  return conn->is_connected && uv_is_readable(s) && uv_is_writable(s);
}

int connection_get_id(const struct connection *conn)
{
  return conn - conn->pool->connections;
}

void connection_resolve_async(struct connection *conn)
{
  struct addrinfo hints;
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  const char* host = conn->config_pool->host;
  const char* port = conn->config_pool->port;

  log_debug("Resolving: %s:%s. Attempt #%d", host, port, conn->resolve_failed_attempts);

  int err_code = uv_getaddrinfo(uv_default_loop(),
                                &conn->resolver,
                                on_getaddrinfo,
                                host, port, &hints);
  if (err_code) {
    connection_resolve_failed(conn, err_code);
  }
}

void connection_resolve_failed(struct connection *conn, int status)
{
  log_error("Error when resolving connection #%d: %s",
            connection_get_id(conn), uv_strerror(status));
  log_debug("Will try again in %d msec", conn->resolve_retry_delay);

  conn->resolve_failed_attempts++;
  // set retry delay
  uv_timer_init(uv_default_loop(), &conn->resolve_retry_timer);
  uv_timer_start(&conn->resolve_retry_timer,
                 on_resolve_retry_timer,
                 conn->resolve_retry_delay,
                 0);

  uint64_t delay = (uint64_t)(conn->resolve_retry_delay * RETRY_DELAY_INCREASE_RATE);
  conn->resolve_retry_delay = delay > MAX_RETRY_DELAY_MILLISEC ? MAX_RETRY_DELAY_MILLISEC : delay;
}

void connection_resolve_success(struct connection *conn, struct addrinfo *res)
{
  conn->addrinfo_current = conn->addrinfo = res;
  // reset failed resolve attempts counter and delay
  conn->resolve_failed_attempts = 0;
  conn->resolve_retry_delay = INITIAL_RETRY_DELAY_MILLISEC;

  connection_log_status(conn);

  connection_connect_async(conn);
}

void connection_connect_async(struct connection *conn)
{
  assert(connection_is_resolved(conn));
  assert(!connection_is_connected(conn));
  assert(conn->addrinfo_current != NULL);

  const struct sockaddr *sockaddr = conn->addrinfo_current->ai_addr;

  uv_tcp_init(uv_default_loop(), &conn->socket);
  assert(!connection_is_connected(conn));

  int err_code = uv_tcp_connect(&conn->connect_req, &conn->socket, sockaddr, on_tcp_connect);
  if(err_code) {
    connection_connect_failed(conn, err_code);
  }
}

void connection_connect_failed(struct connection *conn, int status)
{
  log_error("Error when connecting #%d: %s", connection_get_id(conn), uv_strerror(status));
  conn->is_connected = false;
  // try next addrinfo if any
  if (conn->addrinfo_current->ai_next) {
    log_debug("Trying next address");
    conn->addrinfo_current = conn->addrinfo_current->ai_next;
    connection_connect_async(conn);
  } else {
    // reset connection and resolve again
    log_debug("Resetting connection");
    connection_reset(conn);
    log_debug("Will retry connection again in %d msec", MAX_RETRY_DELAY_MILLISEC);
    // set maximum retry delay
    uv_timer_init(uv_default_loop(), &conn->resolve_retry_timer);
    uv_timer_start(&conn->resolve_retry_timer,
                   on_resolve_retry_timer,
                   MAX_RETRY_DELAY_MILLISEC,
                   0);
  }
}

void connection_connect_success(struct connection *conn)
{
  conn->is_connected = true;
  connection_log_status(conn);
  assert(connection_is_connected(conn));
  pool_connection_switch_active(conn->pool);
}

void connection_log_status(const struct connection *conn)
{
  const char* host = conn->config_pool->host;
  const char* port = conn->config_pool->port;
  log_debug("Connection #%d [%s:%s]: ", connection_get_id(conn), host, port);
  if(connection_is_connected(conn)) {
    char addr[46] = {'\0'};
    assert(connection_is_resolved(conn) && conn->addrinfo_current != NULL);
    log_debug(" + Connected: %s", ip_addr_to_str(conn->addrinfo_current, addr));
  } else if(connection_is_resolved(conn)) {
    log_debug(" + Resolved:");
    for (struct addrinfo *rp = conn->addrinfo; rp != NULL; rp = rp->ai_next) {
      char addr[46] = {'\0'};
      char sym = rp == conn->addrinfo_current ? '*' : '.';
      log_debug("     %c %s", sym, ip_addr_to_str(rp, addr));
    }
  } else {
    log_debug(" - Unresolved");
  }
}


/** Initialize data structures */
bool pool_connection_init(const struct config_pool_list* cfg,
                          pool_connection_handle* handle_ptr)
{
  assert(*handle_ptr == NULL && "Can not initialize non-null handle");

  if(cfg->size == 0) {
    log_error("No hosts configured!");
    return false;
  }

  struct pool_connection *pool = calloc(1, sizeof(struct pool_connection));
  pool->size = cfg->size;
  pool->connections = calloc(cfg->size, sizeof(struct connection));
  pool->connections_end = pool->connections + cfg->size;
  pool->connect_timer.data = pool;

  struct connection *conn = pool->connections;
  for (size_t i = 0; i < cfg->size; ++i, ++conn) {
    conn->pool = pool;
    conn->config_pool = &cfg->pools[i];
    conn->resolve_retry_timer.data = conn->resolver.data = conn->connect_req.data = conn;
    connection_reset(conn);
  }

  *handle_ptr = pool;

  return true;
}

/** it should be called just once per app lifecycle */
void pool_connection_connect(pool_connection_handle handle)
{
  for (struct connection *conn = handle->connections; conn != handle->connections_end; ++conn) {
    assert(!connection_is_resolved(conn));
    connection_resolve_async(conn);
  }
}

bool pool_connection_shutdown(pool_connection_handle handle)
{
  return false;
}

void pool_connection_free(pool_connection_handle handle)
{
  assert(handle != NULL);
  for (struct connection *conn = handle->connections; conn != handle->connections_end; ++conn) {
    connection_reset(conn);
  }
  free(handle->connections);
  free(handle);
}
