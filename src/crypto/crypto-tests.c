#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto/keccak-tiny.h"
#include "crypto/utils.h"

#define UNUSED_ARG(a) ((void)a)
#define DIGEST_LENGTH_BITS 256
#define DIGEST_LENGTH_BYTES (DIGEST_LENGTH_BITS / 8)

struct test_vector {
  const char *msg;
  size_t repeat; // 0 - no repeat (single), 1 - repeat once(double size o the
                 // message), ...
};

// test vectors used in NIST competition
static const struct test_vector NIST_TEST_VECTORS[] = {
    {.msg = "abc", .repeat = 0},
    {.msg = "", .repeat = 0},
    {.msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
     .repeat = 0},
    {.msg = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoij"
            "klmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
     .repeat = 0},
    {.msg = "a", .repeat = 1000000},
    {.msg = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno",
     .repeat = 16777216}};

static const char *KESSAK_256_RESULTS[] = {
    "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532",
    "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a",
    "41c0dba2a9d6240849100376a8235e2c82e1b9998a999e21db32dd97496d3376",
    "916f6061fe879741ca6469b43971dfdb28b1a32dc36cb3254e812be27aad1d18",
    "5c8875ae474a3634ba4fd55ec85bffd661f32aca75c6d699d0cdcb6c115891c1",
    "ecbbc42cbf296603acb2c6bc0410ef4378bafb24b710357f12df607758b33e2b"};

static const size_t NUM_TESTS =
    sizeof(NIST_TEST_VECTORS) / sizeof(struct test_vector);

void do_keccak(const void *msg, size_t msg_len, uint8_t *digest)
{
  sha3_256(digest, DIGEST_LENGTH_BYTES, (uint8_t *)msg, msg_len);
}

typedef void (*hash_fn)(const void *msg, size_t msg_len, uint8_t *digest);

bool test_single(const struct test_vector *test, const char *expected,
                 hash_fn h)
{
  size_t msg_len = strlen(test->msg);
  const char *msg = test->msg;
  if (test->repeat > 0) {
    size_t new_len = msg_len * test->repeat;
    msg = calloc(new_len, 1);
    char *p = (char *)msg;
    for (size_t i = 0; i < test->repeat; ++i, p += msg_len) {
      strncpy(p, test->msg, msg_len);
    }
    msg_len = new_len;
  }
  uint8_t digest[DIGEST_LENGTH_BYTES] = {0};
  h((const uint8_t *)msg, msg_len, digest);
  if (msg != test->msg) {
    free((void *)msg);
  }

  char digest_str[DIGEST_LENGTH_BYTES * 2 + 1];
  bin2hex(digest, DIGEST_LENGTH_BYTES, digest_str);
  if (strncmp(expected, digest_str, DIGEST_LENGTH_BYTES) != 0) {
    printf(" - FAILED: %s <> %s\n", expected, digest_str);
    return false;
  } else {
    printf(" + MATCH: %s\n", expected);
    return true;
  }
}

void test_hash(const char *test_name, const char *expected[], hash_fn h)
{
  printf("Testing %s\n", test_name);
  for (size_t i = 0; i < NUM_TESTS; ++i) {
    test_single(NIST_TEST_VECTORS + i, expected[i], h);
  }
}

int main(int argc, char **argv)
{
  UNUSED_ARG(argc);
  UNUSED_ARG(argv);

  test_hash("Keccak", KESSAK_256_RESULTS, do_keccak);
  assert(false);
  return 1;
}
