#include "buffer.h"

#include "utils/unused.h"
#include <assert.h>
#include <stdlib.h>

/** Generic buffer allocation, needed by uv_read_start */
void buffer_alloc(uv_handle_t *h, size_t suggested_size, uv_buf_t *buf)
{
  UNUSED(h);
  UNUSED(suggested_size);
  assert(suggested_size <= BUFFER_DEFAULT_ALLOC_SIZE);
  *buf = uv_buf_init(calloc(BUFFER_DEFAULT_ALLOC_SIZE, 1),
                     BUFFER_DEFAULT_ALLOC_SIZE - 1);
}
