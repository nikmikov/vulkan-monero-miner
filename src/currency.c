#include "currency.h"

#include <assert.h>
#include <string.h>

#include "logging.h"

static const struct currency_info supported_currencies[] = {
  {CURRENCY_XMR, "Monero", "XMR"},
  {CURRENCY_ETH, "Ethereum", "ETH"},
  {CURRENCY_ZEC, "ZCash", "ZEC"}
};

static const size_t CURRENCIES_SIZE = sizeof(supported_currencies)/sizeof(struct currency_info);

const struct currency_info* currency_get_info(enum currency currency)
{
  size_t idx = (size_t)currency;
  assert(idx < CURRENCIES_SIZE);
  const struct currency_info *info = supported_currencies + idx;
  assert(info->currency == currency);
  return info;
}

bool currency_from_name(const char* str, enum currency *out)
{
  for(size_t i = 0; i < CURRENCIES_SIZE; ++i) {
    log_debug("Comparing: %s with %s", supported_currencies[i].name, str);
    if (strcmp(supported_currencies[i].name, str) == 0) {
      *out = supported_currencies[i].currency;
      return true;
    }
  }
  log_error("Unnown currency name: %s", str);
  return false;
}

bool currency_from_code(const char* str, enum currency *out)
{
  for(size_t i = 0; i < CURRENCIES_SIZE; ++i) {
    if (strcmp(supported_currencies[i].code, str) == 0) {
      *out = supported_currencies[i].currency;
      return true;
    }
  }
  log_error("Unnown currency code: %s", str);
  return false;
}
