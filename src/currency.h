/* currency.h -- Currency utils
 *
 */

#ifndef CURRENCY_H
#define CURRENCY_H

#include <stdbool.h>
#include <stddef.h>

enum currency {
    CURRENCY_XMR,
    CURRENCY_ETH,
    CURRENCY_ZEC
};

struct currency_info {
    enum currency currency;
    const char* name;
    const char* code;
};

const struct currency_info* currency_get_info(enum currency);

bool currency_from_name(const char* str, enum currency* out);

bool currency_from_code(const char* str, enum currency* out);

#endif /** CURRENCY_H */
