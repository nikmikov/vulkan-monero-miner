#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto/blake.h"
#include "crypto/cryptonight/cryptonight.h"
#include "crypto/groestl.h"
#include "crypto/jh.h"
#include "crypto/keccak-tiny.h"
#include "crypto/skein.h"
#include "crypto/utils.h"

#define UNUSED_ARG(a) ((void)a)
#define DIGEST_LENGTH_BITS 256
#define DIGEST_LENGTH_BYTES (DIGEST_LENGTH_BITS / 8)

enum encoding { ENC_STRING, ENC_HEX };

struct test_vector {
  const char *msg;
  // 0 - no repeat (single), 1 - repeat once(double size o the message)
  size_t repeat;
  enum encoding enc;
};

// test vectors used in NIST competition
static const struct test_vector TEST_VECTORS[] = {
    {.msg = "abc", .repeat = 0, .enc = ENC_STRING},
    {.msg = "", .repeat = 0, .enc = ENC_STRING},
    {.msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
     .repeat = 0,
     .enc = ENC_STRING},
    {.msg = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoij"
            "klmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
     .repeat = 0,
     .enc = ENC_STRING},
    {.msg = "a", .repeat = 64, .enc = ENC_STRING},
    {.msg = "a", .repeat = 1000000, .enc = ENC_STRING},
    {.msg = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno",
     .repeat = 16777216,
     .enc = ENC_STRING},
    {.msg = "5318207b35035183b00c1fb8759a893f8ca53b760e5ee63edf807d8ff7d48a02db"
            "de93d49d6857ee6e5e9b876c1770dbd891ecb0a0f58f4f573f519dea4be7d970d0"
            "8e1ad9817285c1ad8407ba102909e090defeac8c3f6b2675ca8df37a01126fe838"
            "63a1c918c7ad5d19ad7147adc8690ce880b7baa9a3af2f2a01c2743ad30b36764a"
            "f9628742d99d8f8d885967655381cfdb1fac8469c7e28ec968cd149cfcd04259ab"
            "2dfb27b797a5fd37d17a023e599dff146968f811a4e8c5af20624ce192b3879ec7"
            "a992",
     .repeat = 0,
     .enc = ENC_HEX}};

static const char *SHA3_256_RESULTS[] = {
    "3a985da74fe225b2045c172d6bd390bd855f086e3e9d525b46bfe24511431532",
    "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a",
    "41c0dba2a9d6240849100376a8235e2c82e1b9998a999e21db32dd97496d3376",
    "916f6061fe879741ca6469b43971dfdb28b1a32dc36cb3254e812be27aad1d18",
    "043d104b5480439c7acff8831ee195183928d9b7f8fcb0c655a086a87923ffee",
    "5c8875ae474a3634ba4fd55ec85bffd661f32aca75c6d699d0cdcb6c115891c1",
    "ecbbc42cbf296603acb2c6bc0410ef4378bafb24b710357f12df607758b33e2b",
    "0b32ff9c1eef7bc4140ee904c232aade39f59f2349c02d25fa5348ddac2ddc66"};

static const char *KECCAK_256_RESULTS[] = {
    "4e03657aea45a94fc7d47ba826c8d667c0d1e6e33a64a036ec44f58fa12d6c45",
    "c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470",
    "45d3b367a6904e6e8d502ee04999a7c27647f91fa845d456525fd352ae3d7371",
    "f519747ed599024f3882238e5ab43960132572b7345fbeb9a90769dafd21ad67",
    "1036d73cc8350b0635393d79759b10488165e792073f84d4462e22edec243b92",
    "fadae6b49f129bbb812be8407b7b2894f34aecf6dbd1f9b0f0c7e9853098fc96",
    "5f313c39963dcf792b5470d4ade9f3a356a3e4021748690a958372e2b06f82a4",
    "42d392674e3fc77247006fb55143a6b179bbe5ababee279ba9f9de1e210ca670"};

static const char *BLAKE_256_RESULTS[] = {
    "1833a9fa7cf4086bd5fda73da32e5a1d75b4c3f89d5c436369f9d78bb2da5c28",
    "716f6e863f744b9ac22c97ec7b76ea5f5908bc5b2f67c61510bfc4751384ea7a",
    "adb13cb0da78463d36fcf40def3f291b3f0673e78127bdb70942cdd640b907b4",
    "8f69d890786569cc878e9995a0ebf5e319746482ab56b8184fec5267190e6ade",
    "84d7f3bbf2cfc3ee940ddb6d25045c6d3f756c4b2077a8128e171d5d165be170",
    "22be6de4aa4214c9403f10598f0a6b0e834570251a13bc27589437f7139a5d44",
    "b2c2ecebdb12cb0193fc18e66e00942a73fff801b4bcd89cc11a7e0e7eea8ae3",
    "14936293d56d3ca3194654e48933607175fd91ada8e3f35b9c56c5c23443af42"};

static const char *SKEIN_256_RESULTS[] = {
    "0977b339c3c85927071805584d5460d8f20da8389bbe97c59b1cfac291fe9527",
    "39ccc4554a8b31853b9de7a1fe638a24cce6b35a55f2431009e18780335d2621",
    "55b077673b26b01842cb2393b2ab4481ce94cdeb1ab56f579f8a2a6937ad5b44",
    "9516ff1e8a8b889294cf6339cea01706a6d8bd582d0c457ced9d740ccffdcf0a",
    "6b8cd8ac4c67fb6468896693b8f5d3bb54002da20901699233b318bbd10fce85",
    "f1904077d9f219b300c49987d2f3b2f1054fea6338a5a327fda1cb02afe218f8",
    "6e44df5b41110404a558111eb4bbd4b088aef7de2d5833171a9695c091f8b9b1",
    "bfabcc134608782e8f7322972dba801267f841535372741b554356045910f614"};

static const char *JH_256_RESULTS[] = {
    "924bc82f24a76d519d4f69493da7fa70dc88bdb6016b6d1cc1dcf7def15e9cdd",
    "46e64619c18bb0a92a5e87185a47eef83ca747b8fcc8e1412921357e326df434",
    "146105745c7e2dfac6a48a9b1223b111fed3a53c3efbfe8c2b2e8fa9cafd94d4",
    "f432ab29a0a72569376d39c4cbb418081c6e114b7890cdf815ea188b0a9428d9",
    "05733727efdd236118340ec8f870689c0c9e571d3ff64614cfea082599e56593",
    "c229c3fcdcbe9fd6e935e80746f31dc76f4241fdc092d9893a1960d59ef1b38e",
    "58ffbde520764dfc03b29598acd70655bb2c245a3d73fdd6eb9e1bc221af579b",
    "8da2b43f907e9c92875200e9df8a996028bf183c5aa1144752927851da7d65d5"};

static const char *GROESTL_256_RESULTS[] = {
    "f3c1bb19c048801326a7efbcf16e3d7887446249829c379e1840d1a3a1e7d4d2",
    "1a52d11d550039be16107f9c58db9ebcc417f16f736adb2502567119f0083467",
    "22c23b160e561f80924d44f2cc5974cd5a1d36f69324211861e63b9b6cb7974c",
    "2538fe0a0ce6e6fee1f5a361c171543bfea6c692e09f160eeb8e10ae97dba4bb",
    "56e6d76870910b6d4258c6f5fdbee846873f94437d6409ab53922b91ce4afe8c",
    "a43cb4311fb1b53e2b207b1345e4e81c4279cf7afc9531ef10fb9edf4e705daf",
    "5f87f9404c1142b9e701076dd047386162213a896560c1656c62bbfedfbeddb6",
    "f7c8149059ada9db84706dd1b2826b992134e76357c8c11502a27ef3b02a4a01"};

static const struct test_vector CRYPTONIGHT_TEST_VECTORS[] = {
    {.msg = "", .repeat = 0, .enc = ENC_STRING},
    {.msg = "This is a test", .repeat = 0, .enc = ENC_STRING},
    {.msg = "This is a test1", .repeat = 0, .enc = ENC_STRING},
    {.msg = "This is a test66", .repeat = 0, .enc = ENC_HEX},
    {.msg = "This is a test6", .repeat = 0, .enc = ENC_STRING},
    {.msg = "0606ebba9cd005f688598a3ad7ae62d6e150005ded336138b26417772375b1bd5d"
            "3c0bc480eeb000000005f3c91e30aab34cbacb1bbb3eecb8b4dfd5e799aa4407b8"
            "a0ea4ee397707bc51017",
     .repeat = 0,
     .enc = ENC_HEX},
};

static const char *CRYPTONIGHT_256_RESULTS[] = {
    "eb14e8a833fac6fe9a43b57b336789c46ffe93f2868452240720607b14387e11",
    "a084f01d1437a09c6985401b60d43554ae105802c5f5d8a9b3253649c0be6605",
    "236ebffb019ea9b19c9ff160775bc7a6ed090fce7103a32ee582d24a81db6960",
    "21fb4137747541810d5f8ce821b6dacf68eb3051778e68b5bcd990c21fa08fd6",
    "bfabcc134608782e8f7322972dba801267f841535372741b554356045910f614",
    "038228f36441187229333dd71d3f1b672335ec526d2101cc5fc700692c6aa9cb"};

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
  blake_256(msg, msg_len * 8, digest);
}

void do_skein(const void *msg, size_t msg_len, uint8_t *digest)
{
  skein_512_256(msg, msg_len * 8, digest);
}

void do_jh(const void *msg, size_t msg_len, uint8_t *digest)
{
  jh_256(msg, msg_len * 8, digest);
}

void do_groestl(const void *msg, size_t msg_len, uint8_t *digest)
{
  groestl_256(msg, msg_len * 8, digest);
}

void do_cryptonight(const void *msg, size_t msg_len, uint8_t *digest)
{
  struct cryptonight_ctx *ctx = cryptonight_ctx_new();
  cryptonight_aesni(msg, msg_len, (struct cryptonight_hash *)digest, ctx);
  cryptonight_ctx_free(&ctx);
}

typedef void (*hash_fn)(const void *msg, size_t msg_len, uint8_t *digest);

int test_single(const char *test_msg, size_t repeat, const char *expected,
                hash_fn h)
{
  size_t msg_len = strlen(test_msg);
  const char *msg = test_msg;
  if (repeat > 0) {
    size_t new_len = msg_len * repeat;
    msg = calloc(new_len, 1);
    char *p = (char *)msg;
    for (size_t i = 0; i < repeat; ++i, p += msg_len) {
      strncpy(p, test_msg, msg_len);
    }
    msg_len = new_len;
  }
  uint8_t digest[DIGEST_LENGTH_BYTES] = {0};
  h((const uint8_t *)msg, msg_len, digest);
  if (msg != test_msg) {
    free((void *)msg);
  }

  char digest_str[DIGEST_LENGTH_BYTES * 2 + 1] = {0};
  bin2hex(digest, DIGEST_LENGTH_BYTES, digest_str);
  if (strncmp(expected, digest_str, DIGEST_LENGTH_BYTES * 2) != 0) {
    printf(" - FAIL: %s <> %s\n", expected, digest_str);
    return 1;
  } else {
    printf(" + PASS: %s\n", expected);
    return 0;
  }
}

int test_hash_fn(const char *test_name, const struct test_vector *vectors,
                 size_t vectors_sz, const char *expected[], hash_fn h)
{
  int failures = 0;
  printf("Testing %s\n", test_name);
  for (size_t i = 0; i < vectors_sz; ++i) {
    const struct test_vector *v = vectors + i;
    switch (v->enc) {
    case ENC_STRING:
      failures += test_single(v->msg, v->repeat, expected[i], h);
      break;
    case ENC_HEX: {
      size_t msglen = strlen(v->msg);
      char *msg = calloc(1, msglen);
      hex2bin(v->msg, msglen, msg);
      failures += test_single(msg, v->repeat, expected[i], h);
      free(msg);
    } break;
    }
  }
  return failures;
}

int test_hash(const char *test_name, const char *expected[], hash_fn h)
{
  return test_hash_fn(test_name, TEST_VECTORS,
                      sizeof(TEST_VECTORS) / sizeof(struct test_vector),
                      expected, h);
}

int test_cryptonight()
{
  return test_hash_fn("Cryptonight", CRYPTONIGHT_TEST_VECTORS,
                      sizeof(CRYPTONIGHT_TEST_VECTORS) /
                          sizeof(struct test_vector),
                      CRYPTONIGHT_256_RESULTS, do_cryptonight);
}

int main(int argc, char **argv)
{
  UNUSED_ARG(argc);
  UNUSED_ARG(argv);

  int failures = 0;
  // failures += test_hash("SHA-3", SHA3_256_RESULTS, do_sha3);
  // failures += test_hash("Keccak", KECCAK_256_RESULTS, do_keccak);
  // failures += test_hash("Blake", BLAKE_256_RESULTS, do_blake);
  // failures += test_hash("Skein-512-256 REF", SKEIN_256_RESULTS,
  // do_skein_ref);  failures += test_hash("Skein-512-256", SKEIN_256_RESULTS,
  // do_skein);  failures += test_hash("JH", JH_256_RESULTS, do_jh);  failures +=
  // test_hash("Groestl", GROESTL_256_RESULTS, do_groestl);
  failures += test_cryptonight();
  if (failures > 0) {
    printf("FAILURE: Tests failed: %d\n", failures);
  } else {
    printf("SUCCESS: All test passed\n");
  }
  return failures;
}
