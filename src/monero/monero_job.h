#ifndef MONERO_JOB_H
#define MONERO_JOB_H

struct monero_job {
  const char *job_id;
  const char *blob;
  const char *target;
};

#endif /** MONERO_JOB */
