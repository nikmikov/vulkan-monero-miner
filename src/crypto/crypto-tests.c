#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto/blake256.h"
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

static const char *SHA3_256_RESULTS[] = {
    "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532",
    "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a",
    "41c0dba2a9d6240849100376a8235e2c82e1b9998a999e21db32dd97496d3376",
    "916f6061fe879741ca6469b43971dfdb28b1a32dc36cb3254e812be27aad1d18",
    "5c8875ae474a3634ba4fd55ec85bffd661f32aca75c6d699d0cdcb6c115891c1",
    "ecbbc42cbf296603acb2c6bc0410ef4378bafb24b710357f12df607758b33e2b"};

static const char *KECCAK_256_RESULTS[] = {
    "4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45",
    "c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470",
    "45d3b367a6904e6e8d502ee04999a7c27647f91fa845d456525fd352ae3d7371",
    "f519747ed599024f3882238e5ab43960132572b7345fbeb9a90769dafd21ad67",
    "fadae6b49f129bbb812be8407b7b2894f34aecf6dbd1f9b0f0c7e9853098fc96",
    "5f313c39963dcf792b5470d4ade9f3a356a3e4021748690a958372e2b06f82a4"};

static const char *BLAKE_256_RESULTS[] = {
    "1833a9fa7cf4086bd5fda73da32e5a1d75b4c3f89d5c436369f9d78bb2da5c28",
    "716f6e863f744b9ac22c97ec7b76ea5f5908bc5b2f67c61510bfc4751384ea7a",
    "adb13cb0da78463d36fcf40def3f291b3f0673e78127bdb70942cdd640b907b4",
    "8f69d890786569cc878e9995a0ebf5e319746482ab56b8184fec5267190e6ade",
    "22be6de4aa4214c9403f10598f0a6b0e834570251a13bc27589437f7139a5d44",
    "b2c2ecebdb12cb0193fc18e66e00942a73fff801b4bcd89cc11a7e0e7eea8ae3"};

static const size_t NUM_TESTS =
    sizeof(NIST_TEST_VECTORS) / sizeof(struct test_vector);

void do_sha3(const void *msg, size_t msg_len, uint8_t *digest)
{
  sha3_256(digest, DIGEST_LENGTH_BYTES, (uint8_t *)msg, msg_len);
}

void do_keccak(const void *msg, size_t msg_len, uint8_t *digest)
{
  keccak_256(digest, DIGEST_LENGTH_BYTES, (uint8_t *)msg, msg_len);
}

void do_blake(const void *msg, size_t msg_len, uint8_t *digest)
{
  blake256_hash(digest, (uint8_t *)msg, msg_len);
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

  char digest_str[DIGEST_LENGTH_BYTES * 2 + 1] = {0};
  bin2hex(digest, DIGEST_LENGTH_BYTES, digest_str);
  if (strncmp(expected, digest_str, DIGEST_LENGTH_BYTES * 2) != 0) {
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

  test_hash("SHA-3", SHA3_256_RESULTS, do_sha3);
  test_hash("Keccak", KECCAK_256_RESULTS, do_keccak);
  test_hash("Blake", BLAKE_256_RESULTS, do_blake);
  assert(false);
  return 1;
}
