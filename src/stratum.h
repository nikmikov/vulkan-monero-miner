/* stratum.h -- generic stratum protocol
 *
 * Workflow:
 * login
 *
 */
#pragma once

#include <uv.h>

#include "config.h"
#include "connection.h"

enum stratum_event_type {
    STRATUM_EVENT_NOEVENT = 0,
    STRATUM_EVENT_INVALID_REPLY,
    STRATUM_EVENT_LOGIN_SUCCESS,
    STRATUM_EVENT_LOGIN_FAILED,
    STRATUM_EVENT_NEW_JOB
};

struct stratum_event {
    enum stratum_event_type event_type;
};

struct stratum_event_invalid_reply {
    struct stratum_event stratum_event;
    const char *error;
};

struct stratum_event_login_success {
    struct stratum_event stratum_event;
};

struct stratum_event_login_failed {
    struct stratum_event stratum_event;
    const char *error;
};

struct stratum_event_new_job {
    struct stratum_event stratum_event;
    void *job_data;
};

struct stratum_event_handler;

typedef struct stratum* stratum_handle;

struct stratum {
    enum stratum_protocol protocol;
    void (*login)(stratum_handle, connection_handle, struct stratum_event_handler*);
    void (*logout)(stratum_handle);
    void (*submit)(stratum_handle, void *data);
    void (*new_payload)(stratum_handle, const uv_buf_t *buf);
};

/** Handle server commands */
typedef void (*stratum_event_cb)(const struct stratum_event *event,
                                 void *data);

struct stratum_event_handler {
    void *data;
    stratum_event_cb cb;
};

stratum_handle stratum_new(const struct config_miner *cfg);

void stratum_free(stratum_handle*);
