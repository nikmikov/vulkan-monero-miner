/*
 * Based on AES-NI Groestl-256
 * Author: Krystian Matusiewicz, Günther A. Roland, Martin Schläffer
 *
 * This code is placed in the public domain
 */
#ifndef CRYPTO_GROESTL_H
#define CRYPTO_GROESTL_H

#include <stdint.h>

void groestl256_hash(uint8_t *out, const uint8_t *in, uint64_t inlen);

#endif
