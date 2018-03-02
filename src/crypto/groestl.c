/*
 * Based on AES-NI Groestl-256
 * Author: Krystian Matusiewicz, Günther A. Roland, Martin Schläffer
 *
 * This code is placed in the public domain
 */
#include "crypto/groestl.h"

#include <assert.h>
#include <stdalign.h>
#include <string.h>

#if defined(__AES__) && defined(__SSSE3__)
#include "crypto/groestl_aesni.h"
#else
#define GROESTL256_GENERIC
#include "crypto/groestl_generic.h"
#endif

/* digest up to len bytes of input (full blocks only) */
static inline void groestl_256_transform(struct groestl_state *state,
                                         const uint8_t *data, uint64_t len)
{
  alignas(16) uint8_t block[GROESTL256_BLOCK_SIZE];
  /* increment block counter */
  state->block_counter += len / GROESTL256_BLOCK_SIZE;

  /* digest message, one block at a time */
  for (; len >= GROESTL256_BLOCK_SIZE;
       len -= GROESTL256_BLOCK_SIZE, data += GROESTL256_BLOCK_SIZE) {
    memcpy(block, data, GROESTL256_BLOCK_SIZE);
    groestl_tf512(state->chaining, block);
  }
}

void groestl_256_init(struct groestl_state *state)
{

  int i;
  for (i = 0; i < GROESTL256_BLOCK_SIZE / 8; ++i) {
    state->chaining[i] = 0;
  }
  for (i = 0; i < GROESTL256_BLOCK_SIZE; ++i) {
    state->buffer[i] = 0;
  }
  /* set initial value */
  // 64-bit BigEndian value of hash length in bits
  // and transpose `chaining` matrix
#ifdef GROESTL256_GENERIC
  state->chaining[7] = 0x001000000000000;
#else
  state->chaining[6] = 0x100000000000000;
#endif

  /* set other variables */
  state->buf_ptr = 0;
  state->block_counter = 0;
  state->bits_in_last_byte = 0;
}

void groestl_256_final(struct groestl_state *state, uint8_t *digest)
{
  int i, j = 0, hashbytelen = GROESTL256_HASH_BIT_LEN / 8;
  uint8_t *s = (uint8_t *)state->chaining;
  /* pad with '1'-bit and first few '0'-bits */
  if (state->bits_in_last_byte) {
    const int BILB = state->bits_in_last_byte;
    state->buffer[state->buf_ptr - 1] &= ((1 << BILB) - 1) << (8 - BILB);
    state->buffer[state->buf_ptr - 1] ^= 0x1 << (7 - BILB);
    state->bits_in_last_byte = 0;
  } else {
    state->buffer[state->buf_ptr++] = 0x80;
  }

  /* pad with '0'-bits */
  if (state->buf_ptr > GROESTL256_BLOCK_SIZE - GROESTL256_ROWS) {
    /* padding requires two blocks */
    while (state->buf_ptr < GROESTL256_BLOCK_SIZE) {
      state->buffer[state->buf_ptr++] = 0;
    }
    /* digest first padding block */
    groestl_256_transform(state, state->buffer, GROESTL256_BLOCK_SIZE);
    state->buf_ptr = 0;
  }

  while (state->buf_ptr < GROESTL256_BLOCK_SIZE - GROESTL256_ROWS) {
    state->buffer[state->buf_ptr++] = 0;
  }

  /* length padding */
  state->block_counter++;
  state->buf_ptr = GROESTL256_BLOCK_SIZE;
  while (state->buf_ptr > GROESTL256_BLOCK_SIZE - GROESTL256_ROWS) {
    state->buffer[--state->buf_ptr] = (uint8_t)state->block_counter;
    state->block_counter >>= 8;
  }

  /* digest final padding block */
  groestl_256_transform(state, state->buffer, GROESTL256_BLOCK_SIZE);
  /* perform output transformation */
  groestl_256_output_transform(state->chaining);

  /* store hash result in output */
  for (i = GROESTL256_BLOCK_SIZE - hashbytelen; i < GROESTL256_BLOCK_SIZE;
       i++, j++) {
    digest[j] = s[i];
  }
}

void groestl_256_update(struct groestl_state *state, const void *dataptr,
                        size_t databitlen)
{
  size_t index = 0;
  size_t msglen = databitlen / 8;
  size_t rem = databitlen % 8;

  const uint8_t *data = dataptr;

  /* if the buffer contains data that has not yet been digested, first
     add data to buffer until full */
  if (state->buf_ptr) {
    while (state->buf_ptr < GROESTL256_BLOCK_SIZE && index < msglen) {
      state->buffer[state->buf_ptr++] = data[index++];
    }
    if (state->buf_ptr < GROESTL256_BLOCK_SIZE) {
      /* buffer still not full, return */
      if (rem) {
        state->bits_in_last_byte = rem;
        state->buffer[state->buf_ptr++] = data[index];
      }
      return;
    }

    /* digest buffer */
    state->buf_ptr = 0;
    groestl_256_transform(state, state->buffer, GROESTL256_BLOCK_SIZE);
  }

  /* digest bulk of message */
  groestl_256_transform(state, data + index, msglen - index);
  index += ((msglen - index) / GROESTL256_BLOCK_SIZE) * GROESTL256_BLOCK_SIZE;
  /* store remaining data in buffer */
  while (index < msglen) {
    state->buffer[state->buf_ptr++] = data[index++];
  }

  /* if non-integral number of bytes have been supplied, store
     remaining bits in last byte, together with information about
     number of bits */
  if (rem) {
    state->bits_in_last_byte = rem;
    state->buffer[state->buf_ptr++] = data[index];
  }
}

void groestl_256(const void *input, size_t inputbitlen, uint8_t *digest)
{
  struct groestl_state state;
  groestl_256_init(&state);
  groestl_256_update(&state, input, inputbitlen);
  groestl_256_final(&state, digest);
}
