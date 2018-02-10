#include "monero/monero_job.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "utils/hex.h"

#define BLOB_SIZE 128

struct monero_job *monero_job_gen_random()
{
  static const size_t blob_int_blocks = 1 + BLOB_SIZE / sizeof(int);

  int *blob = calloc(blob_int_blocks, sizeof(int));
  int *p = blob;
  for (size_t i = 0; i < blob_int_blocks; ++i, ++p) {
    *p = rand();
  }
  char buf[1 + BLOB_SIZE * 2] = {0};
  hex_from_binary(blob, BLOB_SIZE, buf);

  free(blob);
  struct monero_job *job = calloc(1, sizeof(struct monero_job));
  job->job_id = strdup("BenchmarkJob");
  job->target = strdup("0100000000000000");
  job->blob = strdup(buf);
  return job;
}

void monero_job_free(struct monero_job *job)
{
  if (job != NULL) {
    if (job->job_id != NULL) {
      free((void *)job->job_id);
    }
    if (job->blob != NULL) {
      free((void *)job->blob);
    }
    if (job->target != NULL) {
      free((void *)job->target);
    }
    free(job);
  }
}
