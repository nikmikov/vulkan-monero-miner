#ifndef MONERO_RESULT_H
#define MONERO_RESULT_H

struct monero_result {
  const char *job_id;
  const char *nonce;
  const char *hash;
};

#endif /** MONERO_RESULT */
