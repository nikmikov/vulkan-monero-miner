#pragma once

#include "config.h"
#include <stdbool.h>

struct foreman;

typedef struct foreman *foreman_handle;

foreman_handle foreman_new(const struct config *cfg, bool benchmark_mode);

/** free resource and reset handle to NULL*/
void foreman_free(foreman_handle *foreman);

/** start foreman */
void foreman_start(foreman_handle foreman);

/** stop foreman */
void foreman_stop(foreman_handle foreman);
