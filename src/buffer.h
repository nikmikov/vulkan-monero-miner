#ifndef BUFFER_H
#define BUFFER_H

#include <uv.h>

#define BUFFER_DEFAULT_ALLOC_SIZE (64 * 1024)

void buffer_alloc(uv_handle_t *h, size_t suggested_size, uv_buf_t *buf);

#endif /** BUFFER_H */
