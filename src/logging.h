#ifndef LOGGING_H
#define LOGGING_H

#include "console.h"

#define log_error(format, ...) console_log(ERROR, format "\n", ##__VA_ARGS__)
#define log_info(format, ...)  console_log(INFO, format "\n", ## __VA_ARGS__)
#define log_debug(format, ...) console_log(DEBUG,format "\n", ## __VA_ARGS__)

#endif /** LOGGING_H */
