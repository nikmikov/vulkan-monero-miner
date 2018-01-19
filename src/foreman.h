#ifndef FOREMAN_H
#define FOREMAN_H

#include "config.h"

struct foreman;

typedef struct foreman* foreman_handle;

foreman_handle foreman_init(const struct config_miner *cfg);

/** free resource and reset handle to NULL*/
void foreman_free(foreman_handle *foreman);

/** start foreman */
void foreman_start(foreman_handle foreman);

/** stop foreman */
void foreman_stop(foreman_handle foreman);


#endif /** FOREMAN_H */
