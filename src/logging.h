#ifndef LOGGING_H
#define LOGGING_H

#include "console.h"

#define log_error(...) console_log(ERROR, __VA_ARGS__)
#define log_warn(...)  console_log(WARN,  __VA_ARGS__)
#define log_info(...)  console_log(INFO,  __VA_ARGS__)
#define log_debug(...) console_log(DEBUG, __VA_ARGS__)

#endif /** LOGGING_H */
