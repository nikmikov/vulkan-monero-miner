#include "monero/monero_stratum.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON/cJSON.h"

#include "buffer.h"
#include "config.h"

#include "logging.h"
#include "monero/monero_job.h"
#include "monero/monero_result.h"
#include "stratum.h"
#include "version.h"

// json-rpc server response identifier
enum monero_stratum_message_type {
  MONERO_STRATUM_MESSAGE_TYPE_LOGIN = 1,

};

struct monero_stratum {
  struct stratum stratum;
  const char *login;
  const char *password;
  struct stratum_event_handler *stratum_event_handler;
};

/*===================== Utility Functions ======================== */

static inline monero_stratum_handle to_monero_stratum_handle(stratum_handle h)
{
  assert(h != NULL);
  assert(h->protocol == STRATUM_PROTOCOL_MONERO);
  return (monero_stratum_handle)h;
}

static inline const char *json_status_string(const cJSON *json)
{
  assert(json != NULL);
  cJSON *status_json = cJSON_GetObjectItem(json, "status");
  if (status_json == NULL || !cJSON_IsString(status_json)) {
    return NULL;
  }
  return status_json->valuestring;
}

static inline bool json_status_is_ok(const cJSON *json)
{
  const char *status = json_status_string(json);
  if (status == NULL) {
    return false;
  }
  return strcmp(status, "OK") == 0;
}

const char *get_job_string_field(const cJSON *json, const char *field,
                                 struct stratum_event_handler *event_handler)
{
  char err_buf[1024] = {0};
  if (!cJSON_HasObjectItem(json, field)) {
    snprintf(err_buf, 1024, "Unable parse job: field \"%s\" is missing", field);
  } else {
    cJSON *item = cJSON_GetObjectItem(json, field);
    if (!cJSON_IsString(item)) {
      snprintf(err_buf, 1024, "Unable parse job: field \"%s\" is not a string",
               field);
    } else {
      return item->valuestring;
    }
  }
  // error
  assert(err_buf[0] != 0);
  struct stratum_event_invalid_reply event = {
      .stratum_event = {STRATUM_EVENT_INVALID_REPLY}, .error = err_buf};
  event_handler->cb(&event.stratum_event, event_handler->data);
  return NULL;
}

/*========================= Json Response Handling Functions
 * =================================== */
void monero_stratum_handle_json_job(const cJSON *json,
                                    struct stratum_event_handler *event_handler)
{
  log_debug("Processing new job");
  const char *blob, *job_id, *target;
  if ((job_id = get_job_string_field(json, "job_id", event_handler)) == NULL)
    return;
  if ((blob = get_job_string_field(json, "blob", event_handler)) == NULL)
    return;
  if ((target = get_job_string_field(json, "target", event_handler)) == NULL)
    return;

  struct monero_job job = {.job_id = job_id, .blob = blob, .target = target};

  struct stratum_event_new_job event = {
      .stratum_event = {STRATUM_EVENT_NEW_JOB}, .job_data = &job};

  event_handler->cb(&event.stratum_event, event_handler->data);
}

void monero_stratum_handle_json_login_response(
    const cJSON *json, const char *err_msg,
    struct stratum_event_handler *event_handler)
{
  cJSON *miner_id_json = cJSON_GetObjectItem(json, "id");
  cJSON *job_json = cJSON_GetObjectItem(json, "job");

  if (err_msg != NULL) {
    struct stratum_event_login_failed event = {
        .stratum_event = {STRATUM_EVENT_LOGIN_FAILED}, .error = err_msg};
    event_handler->cb(&event.stratum_event, event_handler->data);
  } else if (!json_status_is_ok(json)) {
    struct stratum_event_login_failed event = {
        .stratum_event = {STRATUM_EVENT_LOGIN_FAILED},
        .error = "Server returned status is not \"OK\", but error message was "
                 "not provided"};
    event_handler->cb(&event.stratum_event, event_handler->data);
  } else if (miner_id_json == NULL) {
    struct stratum_event_invalid_reply event = {
        .stratum_event = {STRATUM_EVENT_INVALID_REPLY},
        .error = "Failed to parse server reply: expected miner \"id\" field is "
                 "missing"};
    event_handler->cb(&event.stratum_event, event_handler->data);
  } else {
    log_debug("Miner id: %s", miner_id_json->valuestring);
    struct stratum_event_login_success event = {
        .stratum_event = {STRATUM_EVENT_LOGIN_SUCCESS}};

    event_handler->cb(&event.stratum_event, event_handler->data);
    if (job_json != NULL) {
      monero_stratum_handle_json_job(job_json, event_handler);
    }
  }
}

/** Handle json-rpc response server */
void monero_stratum_handle_json_response(
    const cJSON *json, struct stratum_event_handler *event_handler)
{
  cJSON *id_json = cJSON_GetObjectItem(json, "id");
  cJSON *error_json = cJSON_GetObjectItem(json, "error");
  cJSON *result_json = cJSON_GetObjectItem(json, "result");

  const char *err_msg = NULL;
  if (error_json != NULL && !cJSON_IsNull(error_json)) {
    cJSON *err_json = cJSON_GetObjectItem(error_json, "message");
    err_msg = (err_json != NULL && cJSON_IsString(err_json))
                  ? err_json->valuestring
                  : "Server returned unparsable error";
    log_debug("Server returned error: %s", err_msg);
  }

  if (id_json == NULL) {
    struct stratum_event_invalid_reply event = {
        .stratum_event = {STRATUM_EVENT_INVALID_REPLY},
        .error = "Expected \"id\" field in server response"};
    event_handler->cb(&event.stratum_event, event_handler->data);
    return;
  }

  switch (id_json->valueint) {
  case MONERO_STRATUM_MESSAGE_TYPE_LOGIN:
    monero_stratum_handle_json_login_response(result_json, err_msg,
                                              event_handler);
    break;
  default:
    log_error("Unregistered response id: %d", id_json->valueint);
    assert(false);
  }
}

/** Handle json-rpc request sent by server */
void monero_stratum_handle_json_request(
    const char *method, const cJSON *params_json,
    struct stratum_event_handler *event_handler)
{
  if (strcmp(method, "job")) {
    log_debug("Received job request");
    monero_stratum_handle_json_job(params_json, event_handler);
  } else {
    log_error("Unsupported method: %s", method);
  }
}

/** Handle json-rpc message from server */
void monero_stratum_handle_json(const cJSON *json,
                                struct stratum_event_handler *event_handler)
{
  cJSON *method_json = cJSON_GetObjectItem(json, "method");
  if (method_json != NULL && cJSON_IsString(method_json)) {
    const char *method = method_json->valuestring;
    monero_stratum_handle_json_request(
        method, cJSON_GetObjectItem(json, "params"), event_handler);
  } else {
    monero_stratum_handle_json_response(json, event_handler);
  }
}

void monero_stratum_login(stratum_handle stratum, connection_handle connection,
                          struct stratum_event_handler *event_handler)
{
  monero_stratum_handle monero_stratum = to_monero_stratum_handle(stratum);
  uv_buf_t buf;
  buffer_alloc(NULL, BUFFER_DEFAULT_ALLOC_SIZE, &buf);
  const char *login_cmd =
      "{\"id\":%d,\"jsonrpc\":\"2.0\",\"method\":\"login\",\"params\":{"
      "\"login\":\"%s\","
      "\"pass\":\"%s\","
      "\"agent\":\"%s\""
      "}}\n";

  int len =
      snprintf(buf.base, buf.len, login_cmd, MONERO_STRATUM_MESSAGE_TYPE_LOGIN,
               monero_stratum->login, monero_stratum->password, VERSION_LONG);

  log_debug("Prepared login command(sz: %d): %s", len, buf.base);
  if (len < 0) {
    // error
    log_error("TODO: handle error");
    return;
  }
  assert((size_t)len < buf.len);
  buf.len = (size_t)len;
  monero_stratum->stratum_event_handler = event_handler;
  connection_write(connection, buf);
}

void monero_stratum_logout(stratum_handle stratum) { log_info("Logging out"); }

void monero_stratum_submit(stratum_handle stratum, void *data)
{
  struct monero_result *result = data;
}

void monero_stratum_new_payload(stratum_handle stratum, const uv_buf_t *buf)
{
  log_debug("New payload received.");
  monero_stratum_handle monero_stratum = to_monero_stratum_handle(stratum);
  assert(buf != NULL && buf->len > 0);
  assert(monero_stratum->stratum_event_handler != NULL);

  log_debug("Parsing server json response.");
  cJSON *json = cJSON_Parse(buf->base);

  if (json == NULL) {
    log_error("Failed to parse json response from the server");
    struct stratum_event_invalid_reply event = {
        .stratum_event = {STRATUM_EVENT_INVALID_REPLY},
        .error = "Failed to parse server reply: Invalid json"};
    monero_stratum->stratum_event_handler->cb(
        &event.stratum_event, monero_stratum->stratum_event_handler->data);
    return;
  }
  log_debug("Parsing server json response. Success");
  monero_stratum_handle_json(json, monero_stratum->stratum_event_handler);
  cJSON_Delete(json);
}

monero_stratum_handle monero_stratum_new(const char *login,
                                         const char *password)
{
  assert(login != NULL);
  struct monero_stratum *monero_stratum =
      calloc(1, sizeof(struct monero_stratum));
  stratum_handle stratum = &monero_stratum->stratum;
  stratum->protocol = STRATUM_PROTOCOL_MONERO;

  stratum->login = monero_stratum_login;
  stratum->logout = monero_stratum_logout;
  stratum->submit = monero_stratum_submit;
  stratum->new_payload = monero_stratum_new_payload;

  monero_stratum->login = strdup(login != NULL ? login : "");
  monero_stratum->password = strdup(password != NULL ? password : "");

  return monero_stratum;
}

void monero_stratum_free(monero_stratum_handle *stratum)
{
  monero_stratum_handle handle = *stratum;
  if (handle->login) {
    free((void *)handle->login);
  }
  if (handle->password) {
    free((void *)handle->password);
  }
  free(handle);
  *stratum = NULL;
}
