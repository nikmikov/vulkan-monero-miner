#include "console.h"
#include <stdarg.h>
#include <stdio.h>

void console_init(const struct console_config* cfg) {

}

void console_log(enum log_level log_level, const char* fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);
    vfprintf(stderr, fmt, vargs);
    va_end(vargs);
}
