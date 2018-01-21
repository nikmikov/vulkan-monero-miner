#pragma once

#include "config.h"

enum miner_event_type {
    MINER_EVENT_RESULT_FOUND,
    MINER_EVENT_SEARCH_SPACE_EXHAUSTED
};

struct miner_event {
    enum miner_event_type event_type;
};

struct miner_event_result_found {
    enum miner_event_type event_type;
    void *data;
};

/** Handle server commands */
typedef void (*miner_event_cb)(const struct miner_event *event,
                                void *data);

struct miner_event_handler {
    void *data;
    miner_event_cb cb;
};

typedef struct miner* miner_handle;

struct miner {
    void (*new_job)(miner_handle, void *job_data, struct miner_event_handler *event_handler);
    void (*free)(miner_handle*);
};


miner_handle miner_new(const struct config_miner *cfg);
