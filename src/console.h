/* console.h -- console interfaces
 *
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>

enum log_level { ERROR, WARN, INFO, DEBUG };

/** */
struct console_config{
    bool no_color;
    enum log_level log_level;
    const char* log_format;
};

void console_init(const struct console_config*);
void console_log(enum log_level, const char *logger_name, const char *format, ...);

#endif /** CONSOLE_H */
