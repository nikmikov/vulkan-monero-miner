#ifndef WORKER_H
#define WORKER_H

#include "config.h"

enum worker_event_type {
    WORKER_EVENT_TYPE_RESULT_FOUND,
    WORKER_EVENT_SEARCH_SPACE_EXHAUSTED
};

struct worker_event {
    enum worker_event_type event_type;
};

/** Handle server commands */
typedef void (*worker_event_cb)(const struct worker_event *event,
                                void *data);

struct worker_event_handler {
    void *data;
    worker_event_cb cb;
};

typedef struct worker* worker_handle;

struct worker {
    void (*new_job)(worker_handle, void *job_data, struct worker_event_handler *event_handler);
    void (*free)(worker_handle*);
};


worker_handle worker_new(const struct config_miner *cfg);


#endif /** WORKER_H */
