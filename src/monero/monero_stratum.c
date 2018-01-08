#include "monero/monero_stratum.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"
#include "config.h"
#include "logging.h"
#include "stratum.h"
#include "version.h"

struct monero_stratum {
  struct stratum stratum;
  const char *login;
  const char *password;
};

static inline monero_stratum_handle to_monero_stratum_handle(stratum_handle h)
{
  assert(h->protocol == STRATUM_PROTOCOL_MONERO);
  return (monero_stratum_handle)h;
}


void monero_stratum_login(stratum_handle stratum,
                          connection_handle connection,
                          struct stratum_event_handler *event_handler)
{
  monero_stratum_handle monero_stratum = to_monero_stratum_handle(stratum);
  uv_buf_t buf;
  buffer_alloc(NULL, BUFFER_DEFAULT_ALLOC_SIZE, &buf);
  const char *login_cmd = "{\"id\":1,\"method\":\"login\",\"params\":{"
    "\"login\":\"%s\","
    "\"pass\":\"%s\","
    "\"agent\":\"%s\""
    "}}\n";

  int len = snprintf(buf.base, buf.len, login_cmd,
                     monero_stratum->login,
                     monero_stratum->password,
                     VERSION_LONG);

  log_debug("Prepared login command(sz: %d): %s", len, buf.base);
  if (len < 0) {
    // error
    log_error("TODO: handle error");
    return;
  }
  assert((size_t)len < buf.len);
  buf.len = (size_t)len;

  connection_write(connection, buf);


}

void monero_stratum_logout(stratum_handle stratum)
{

}

void monero_stratum_submit(stratum_handle stratum, void *data)
{

}

monero_stratum_handle monero_stratum_new(const char *login, const char *password)
{
  assert(login != NULL);
  struct monero_stratum *monero_stratum = calloc(1, sizeof(struct monero_stratum));
  stratum_handle stratum = &monero_stratum->stratum;
  stratum->protocol = STRATUM_PROTOCOL_MONERO;

  stratum->login = monero_stratum_login;
  stratum->logout = monero_stratum_logout;
  stratum->submit = monero_stratum_submit;

  monero_stratum->login = strdup(login != NULL ? login : "");
  monero_stratum->password = strdup(password != NULL ? password : "");

  return monero_stratum;
}

void monero_stratum_free(monero_stratum_handle *stratum)
{
  monero_stratum_handle handle = *stratum;
  if(handle->login) {
    free((void*)handle->login);
  }
  if(handle->password) {
    free((void*)handle->password);
  }
  free(handle);
  *stratum = NULL;
}
