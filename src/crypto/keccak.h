// keccak.h
// 19-Nov-11  Markku-Juhani O. Saarinen <mjos@iki.fi>
#pragma once

#include <stdint.h>
#include <string.h>

// compute a keccak hash (md) of given byte length from "in"
int keccak(const uint8_t *in, int inlen, uint8_t *md, int mdlen);

// update the state
void keccakf(uint64_t st[25], int norounds);
