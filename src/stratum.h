/* stratum.h -- generic stratum protocol
 *
 * Workflow:
 * login
 *
 */
#ifndef STRATUM_H
#define STRATUM_H

#include <uv.h>

#include "config.h"
#include "connection.h"

enum stratum_event_type {
    STRATUM_EVENT_NOEVENT = 0,
    STRATUM_EVENT_LOGIN_RESULT,
    STRATUM_EVENT_NEW_JOB
};

struct stratum_payload;
struct stratum_event_handler;


typedef struct stratum* stratum_handle;
typedef struct stratum_payload* stratum_payload_handle;

struct stratum {
    enum stratum_protocol protocol;
    void (*login)(stratum_handle, connection_handle, struct stratum_event_handler*);
    void (*logout)(stratum_handle);
    void (*submit)(stratum_handle, void *data);
    void (*new_payload)(stratum_handle, const uv_buf_t *buf);
};



/** Handle server commands */
typedef void (*stratum_event_cb)(enum stratum_event_type event,
                                 stratum_payload_handle payload,
                                 void *data);

struct stratum_event_handler {
    void *data;
    stratum_event_cb cb;
};

stratum_handle stratum_new(const struct config_miner *cfg);

void stratum_free(stratum_handle*);


#endif /** STRATUM_H */
