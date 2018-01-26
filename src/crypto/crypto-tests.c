#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto/blake256.h"
#include "crypto/jh256.h"
#include "crypto/groestl.h"
#include "crypto/skein256.h"
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

static const char *SKEIN_256_RESULTS[] = {
  "0977b339c3c85927071805584d5460d8f20da8389bbe97c59b1cfac291fe9527",
  "39ccc4554a8b31853b9de7a1fe638a24cce6b35a55f2431009e18780335d2621",
  "55b077673b26b01842cb2393b2ab4481ce94cdeb1ab56f579f8a2a6937ad5b44",
  "9516ff1e8a8b889294cf6339cea01706a6d8bd582d0c457ced9d740ccffdcf0a",
  "f1904077d9f219b300c49987d2f3b2f1054fea6338a5a327fda1cb02afe218f8",
  "6e44df5b41110404a558111eb4bbd4b088aef7de2d5833171a9695c091f8b9b1"
};

static const char *JH_256_RESULTS[] = {
  "924bc82f24a76d519d4f69493da7fa70dc88bdb6016b6d1cc1dcf7def15e9cdd",
  "46e64619c18bb0a92a5e87185a47eef83ca747b8fcc8e1412921357e326df434",
  "146105745c7e2dfac6a48a9b1223b111fed3a53c3efbfe8c2b2e8fa9cafd94d4",
  "f432ab29a0a72569376d39c4cbb418081c6e114b7890cdf815ea188b0a9428d9",
  "c229c3fcdcbe9fd6e935e80746f31dc76f4241fdc092d9893a1960d59ef1b38e",
  "58ffbde520764dfc03b29598acd70655bb2c245a3d73fdd6eb9e1bc221af579b"
};

static const char *GROESTL_256_RESULTS[] = {
  "f3c1bb19c048801326a7efbcf16e3d7887446249829c379e1840d1a3a1e7d4d2",
  "1a52d11d550039be16107f9c58db9ebcc417f16f736adb2502567119f0083467",
  "22c23b160e561f80924d44f2cc5974cd5a1d36f69324211861e63b9b6cb7974c",
  "2538fe0a0ce6e6fee1f5a361c171543bfea6c692e09f160eeb8e10ae97dba4bb",
  "a43cb4311fb1b53e2b207b1345e4e81c4279cf7afc9531ef10fb9edf4e705daf",
  "5f87f9404c1142b9e701076dd047386162213a896560c1656c62bbfedfbeddb6"
};

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

void do_skein(const void *msg, size_t msg_len, uint8_t *digest)
{
  skein256_hash(msg, msg_len, digest);
}

void do_jh(const void *msg, size_t msg_len, uint8_t *digest)
{
  jh256_hash(msg, msg_len, digest);
}

void do_groestl(const void *msg, size_t msg_len, uint8_t *digest)
{
  groestl_256(msg, msg_len * 8, digest);
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

  //test_hash("SHA-3", SHA3_256_RESULTS, do_sha3);
  //test_hash("Keccak", KECCAK_256_RESULTS, do_keccak);
  //test_hash("Blake", BLAKE_256_RESULTS, do_blake);
  //test_hash("Skein", SKEIN_256_RESULTS, do_skein);
  //test_hash("JH", JH_256_RESULTS, do_jh);
  test_hash("Groestl", GROESTL_256_RESULTS, do_groestl);
  assert(false);
  return 1;
}