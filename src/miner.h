#ifndef MINER_H
#define MINER_H

#include "config.h"

struct miner;

typedef struct miner* miner_handle;

miner_handle miner_init(const struct config_miner *cfg);

/** free resource and reset handle to NULL*/
void miner_free(miner_handle *miner);

/** start miner */
void miner_start(miner_handle miner);

/** stop miner */
void miner_stop(miner_handle miner);


#endif /** MINER_H */
