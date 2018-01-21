#include "connection.h"

#include <assert.h>
#include <stdlib.h>

#include <uv.h>

#include "buffer.h"
#include "logging.h"

#define INITIAL_RETRY_DELAY_MILLISEC (1 * 1000)
#define MAX_RETRY_DELAY_MILLISEC (5 * 60 * 1000) // 5 minutes
#define RETRY_DELAY_INCREASE_RATE 2.7182818

struct tcp_connection {
  struct connection *pool;
  /** const data from config */
  const struct config_pool *config_pool;

  //////////////     DNS RESOLVE
  // DNS resolve handle
  uv_getaddrinfo_t resolver;

  // resolved address structure
  struct addrinfo *addrinfo;

  // pointer to current addrinfo, will move to addrinfo->next with each
  // unsuccessfull connect
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

struct connection {
  // number of connections
  size_t size;
  // pointer to first connection
  struct tcp_connection *connections;
  // pointer to the end of connection array
  struct tcp_connection *connections_end;
  // Current active connection or NULL
  struct tcp_connection *active;
  // event handler
  struct connection_event_handler *connection_event_handler;
};

typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

void connection_switch_active(struct connection *pool);

/** reset connection to it's initial state, freeing resources when necessary and
 * closing handles */
void tcp_connection_reset(struct tcp_connection *conn);
/** return true if connection address is resolved */
bool tcp_connection_is_resolved(const struct tcp_connection *conn);
/** return true if connection is open  */
bool tcp_connection_is_connected(const struct tcp_connection *conn);
/** return true if connection is open  */
int tcp_connection_get_id(const struct tcp_connection *conn);
/** asynchronous DNS resolution */
void tcp_connection_resolve_async(struct tcp_connection *conn);
/** if getaddrinfo failed, retry after a delay */
void tcp_connection_resolve_failed(struct tcp_connection *conn, int status);
/** if getaddrinfo successfull, initiate TCP connection */
void tcp_connection_resolve_success(struct tcp_connection *conn,
                                    struct addrinfo *res);
/** try to establish TCP connection asynchronously */
void tcp_connection_connect_async(struct tcp_connection *conn);
/** unable to establish TCP connection */
void tcp_connection_connect_failed(struct tcp_connection *conn, int status);
/** connection successful */
void tcp_connection_connect_success(struct tcp_connection *conn);
/** print connection status to debug log */
void tcp_connection_log_status(const struct tcp_connection *conn);

/* ============    Utility Functions    ============== */
const char *ip_addr_to_str(const struct addrinfo *rp, char *buf)
{
  switch (rp->ai_family) {
  case AF_INET:
    uv_ip4_name((struct sockaddr_in *)rp->ai_addr, buf, 16);
    break;
  case AF_INET6:
    uv_ip6_name((struct sockaddr_in6 *)rp->ai_addr, buf, 45);
    break;
  }
  return buf;
}

/* ============       Callbacks         ============== */
void on_getaddrinfo(uv_getaddrinfo_t *resolver, int status,
                    struct addrinfo *res)
{
  struct tcp_connection *conn = resolver->data;
  assert(conn->addrinfo == NULL && "Expect address is not resolved yet");

  if (status < 0) {
    tcp_connection_resolve_failed(conn, status);
  } else {
    tcp_connection_resolve_success(conn, res);
  }
}

void on_resolve_retry_timer(uv_timer_t *handle)
{
  tcp_connection_resolve_async(handle->data);
}

void on_tcp_connect(uv_connect_t *req, int status)
{
  struct tcp_connection *conn = req->data;
  if (status < 0) {
    tcp_connection_connect_failed(conn, status);
  } else {
    assert((void *)req->handle == (void *)&conn->socket);
    tcp_connection_connect_success(conn);
  }
}

void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
  if (nread < 0) {
    if (nread != UV_EOF) {
      // log_error("Error on reading server stream: %s.",
      // uv_strerror(uv_last_error(loop)));
    }

    uv_close((uv_handle_t *)stream, NULL);
  } else if (nread > 0) {
    log_debug("Read: %s", buf->base);
    assert(stream->data != NULL);
    struct connection *conn = stream->data;
    if (conn->connection_event_handler != NULL) {
      conn->connection_event_handler->cb(CONNECTION_EVENT_DATA, buf,
                                         conn->connection_event_handler->data);
    }
  }

  free(buf->base);
}

void on_write(uv_write_t *req, int status)
{
  if (status < 0) {
    log_error("Error writing to socket; %s", uv_strerror(status));
  }
  log_debug("Writing to socket. Complete");
  write_req_t *wr = (write_req_t *)req;
  free(wr->buf.base);
  free(wr);
}

/* ============  Connection Functions   ============== */
void tcp_connection_reset(struct tcp_connection *conn)
{
  log_debug("Resetting connection #%d", tcp_connection_get_id(conn));
  uv_cancel((uv_req_t *)&conn->resolver);
  uv_timer_stop(&conn->resolve_retry_timer);
  if (conn->is_connected && !uv_is_closing((uv_handle_t *)&conn->socket)) {
    uv_close((uv_handle_t *)&conn->socket, NULL);
  }
  uv_freeaddrinfo(conn->addrinfo);
  conn->addrinfo = conn->addrinfo_current = NULL;

  conn->resolve_retry_delay = INITIAL_RETRY_DELAY_MILLISEC;
  conn->resolve_failed_attempts = 0;
  conn->is_connected = false;
}

bool tcp_connection_is_resolved(const struct tcp_connection *conn)
{
  return conn->addrinfo != NULL;
}

bool tcp_connection_is_connected(const struct tcp_connection *conn)
{
  const uv_stream_t *s = (const uv_stream_t *)&conn->socket;
  return conn->is_connected && uv_is_readable(s) && uv_is_writable(s);
}

int tcp_connection_get_id(const struct tcp_connection *conn)
{
  return conn - conn->pool->connections;
}

void tcp_connection_resolve_async(struct tcp_connection *conn)
{
  struct addrinfo hints;
  hints.ai_family = PF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = 0;

  const char *host = conn->config_pool->host;
  const char *port = conn->config_pool->port;

  log_debug("Resolving: %s:%s. Attempt #%d", host, port,
            conn->resolve_failed_attempts);

  int err_code = uv_getaddrinfo(uv_default_loop(), &conn->resolver,
                                on_getaddrinfo, host, port, &hints);
  if (err_code) {
    tcp_connection_resolve_failed(conn, err_code);
  }
}

void tcp_connection_resolve_failed(struct tcp_connection *conn, int status)
{
  log_error("Error when resolving connection #%d: %s",
            tcp_connection_get_id(conn), uv_strerror(status));
  log_debug("Will try again in %d msec", conn->resolve_retry_delay);

  conn->resolve_failed_attempts++;
  // set retry delay
  uv_timer_init(uv_default_loop(), &conn->resolve_retry_timer);
  uv_timer_start(&conn->resolve_retry_timer, on_resolve_retry_timer,
                 conn->resolve_retry_delay, 0);

  uint64_t delay =
      (uint64_t)(conn->resolve_retry_delay * RETRY_DELAY_INCREASE_RATE);
  conn->resolve_retry_delay =
      delay > MAX_RETRY_DELAY_MILLISEC ? MAX_RETRY_DELAY_MILLISEC : delay;
}

void tcp_connection_resolve_success(struct tcp_connection *conn,
                                    struct addrinfo *res)
{
  conn->addrinfo_current = conn->addrinfo = res;
  // reset failed resolve attempts counter and delay
  conn->resolve_failed_attempts = 0;
  conn->resolve_retry_delay = INITIAL_RETRY_DELAY_MILLISEC;

  tcp_connection_log_status(conn);

  tcp_connection_connect_async(conn);
}

void tcp_connection_connect_async(struct tcp_connection *conn)
{
  assert(tcp_connection_is_resolved(conn));
  assert(!tcp_connection_is_connected(conn));
  assert(conn->addrinfo_current != NULL);

  const struct sockaddr *sockaddr = conn->addrinfo_current->ai_addr;

  uv_tcp_init(uv_default_loop(), &conn->socket);
  assert(!tcp_connection_is_connected(conn));

  int err_code = uv_tcp_connect(&conn->connect_req, &conn->socket, sockaddr,
                                on_tcp_connect);
  if (err_code) {
    tcp_connection_connect_failed(conn, err_code);
  }
}

void tcp_connection_connect_failed(struct tcp_connection *conn, int status)
{
  log_error("Error when connecting #%d: %s", tcp_connection_get_id(conn),
            uv_strerror(status));
  conn->is_connected = false;
  // try next addrinfo if any
  if (status == UV_ECANCELED) {
    return;
  } else if (conn->addrinfo_current->ai_next) {
    log_debug("Trying next address");
    conn->addrinfo_current = conn->addrinfo_current->ai_next;
    tcp_connection_connect_async(conn);
  } else {
    // reset connection and resolve again
    log_debug("Resetting connection");
    tcp_connection_reset(conn);
    log_debug("Will retry connection again in %d msec",
              MAX_RETRY_DELAY_MILLISEC);
    // set maximum retry delay
    uv_timer_init(uv_default_loop(), &conn->resolve_retry_timer);
    uv_timer_start(&conn->resolve_retry_timer, on_resolve_retry_timer,
                   MAX_RETRY_DELAY_MILLISEC, 0);
  }
}

void tcp_connection_connect_success(struct tcp_connection *conn)
{
  conn->is_connected = true;
  tcp_connection_log_status(conn);
  assert(tcp_connection_is_connected(conn));
  connection_switch_active(conn->pool);
}

void tcp_connection_log_status(const struct tcp_connection *conn)
{
  const char *host = conn->config_pool->host;
  const char *port = conn->config_pool->port;
  log_debug("Connection #%d [%s:%s]: ", tcp_connection_get_id(conn), host,
            port);
  if (tcp_connection_is_connected(conn)) {
    char addr[46] = {'\0'};
    assert(tcp_connection_is_resolved(conn) && conn->addrinfo_current != NULL);
    log_debug(" + Connected: %s", ip_addr_to_str(conn->addrinfo_current, addr));
  } else if (tcp_connection_is_resolved(conn)) {
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
connection_handle connection_init(const struct config_pool_list *cfg)
{
  if (cfg->size == 0) {
    log_error("No hosts configured!");
    return NULL;
  }

  struct connection *pool = calloc(1, sizeof(struct connection));
  pool->size = cfg->size;
  pool->connections = calloc(cfg->size, sizeof(struct tcp_connection));
  pool->connections_end = pool->connections + cfg->size;
  struct tcp_connection *conn = pool->connections;
  for (size_t i = 0; i < cfg->size; ++i, ++conn) {
    conn->pool = pool;
    conn->config_pool = &cfg->pools[i];
    conn->resolve_retry_timer.data = conn->resolver.data =
        conn->connect_req.data = conn;
    tcp_connection_reset(conn);
    conn->socket.data = pool;
  }

  return pool;
}

/** it should be called just once per app lifecycle */
void connection_start(connection_handle handle,
                      struct connection_event_handler *event_handler)
{
  handle->connection_event_handler = event_handler;
  for (struct tcp_connection *conn = handle->connections;
       conn != handle->connections_end; ++conn) {
    assert(!tcp_connection_is_resolved(conn));
    tcp_connection_resolve_async(conn);
  }
}

void connection_stop(connection_handle handle)
{
  assert(handle != NULL && handle->connections != NULL);
  if (handle->connection_event_handler) {
    handle->connection_event_handler = NULL;
  }
  for (struct tcp_connection *conn = handle->connections;
       conn != handle->connections_end; ++conn) {
    tcp_connection_reset(conn);
  }
}

void connection_write(connection_handle handle, uv_buf_t data)
{
  assert(handle != NULL && handle->active != NULL);
  uv_stream_t *s = (uv_stream_t *)&handle->active->socket;
  write_req_t *req = calloc(1, sizeof(write_req_t));
  req->buf = data;
  log_debug("Writing to socket");
  uv_write((uv_write_t *)req, s, &req->buf, 1, on_write);
}

void connection_free(connection_handle *handle)
{
  free((*handle)->connections);
  free(*handle);
  *handle = NULL;
}

/** switch active connection */
void connection_switch_active(struct connection *pool)
{
  for (struct tcp_connection *conn = pool->connections;
       conn != pool->connections_end; ++conn) {
    if (tcp_connection_is_connected(conn)) {
      if (conn != pool->active) {
        if (pool->active == NULL) {
          log_debug("Setting active connection to: %d",
                    tcp_connection_get_id(conn));
        } else {
          log_debug("Switching active connection %d => %d",
                    tcp_connection_get_id(pool->active),
                    tcp_connection_get_id(conn));

          // stopping read
          uv_read_stop((uv_stream_t *)&pool->active->socket);
        }
        pool->active = conn;
        if (pool->connection_event_handler) {
          // notify callback
          assert(pool->connection_event_handler->cb != NULL);
          pool->connection_event_handler->cb(
              CONNECTION_EVENT_CONNECTED, NULL,
              pool->connection_event_handler->data);
        }
        uv_read_start((uv_stream_t *)&pool->active->socket, buffer_alloc,
                      on_read);
      }
      return;
    }
  }
}
