#pragma once

struct monero_job {
  const char *job_id;
  const char *blob;
  const char *target;
};

/** Generate random job for benchmarking and testing */
struct monero_job *monero_job_gen_random();

void monero_job_free(struct monero_job *);
