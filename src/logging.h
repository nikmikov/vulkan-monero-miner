#pragma once

#include "console.h"

#define log_error(...) console_log(ERROR, __FILE__, __VA_ARGS__)
#define log_warn(...) console_log(WARN, __FILE__, __VA_ARGS__)
#define log_info(...) console_log(INFO, __FILE__, __VA_ARGS__)
#define log_debug(...) console_log(DEBUG, __FILE__, __VA_ARGS__)
