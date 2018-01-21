#pragma once

struct monero_stratum;
typedef struct monero_stratum *monero_stratum_handle;

monero_stratum_handle monero_stratum_new(const char *login,
                                         const char *password);

void monero_stratum_free(monero_stratum_handle *);
