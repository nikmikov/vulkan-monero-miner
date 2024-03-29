#include "console.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_GRAY "\x1b[37m"
#define ANSI_COLOR_LIGHT_YELLOW "\x1b[93m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define ERROR_PREFIX ANSI_COLOR_RED "ERROR" ANSI_COLOR_RESET ": "
#define INFO_PREFIX ANSI_COLOR_GREEN "INFO " ANSI_COLOR_RESET ": "
#define WARN_PREFIX ANSI_COLOR_MAGENTA "WARN " ANSI_COLOR_RESET ": "
#define DEBUG_PREFIX ANSI_COLOR_LIGHT_YELLOW "DEBUG" ANSI_COLOR_RESET ": "

void console_init(const struct console_config *cfg) {}

void console_log(enum log_level log_level, const char *logger_name,
                 const char *fmt, ...)
{
  switch (log_level) {
  case DEBUG:
    fprintf(stderr, DEBUG_PREFIX);
    break;
  case INFO:
    fprintf(stderr, INFO_PREFIX);
    break;
  case WARN:
    fprintf(stderr, WARN_PREFIX);
    break;
  case ERROR:
    fprintf(stderr, ERROR_PREFIX);
    break;
  }

  char buff[20];
  struct tm *sTm;

  time_t now = time(0);
  sTm = gmtime(&now);

  strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);

  fprintf(stderr, "%s %s: ", buff, logger_name);
  va_list vargs;
  va_start(vargs, fmt);
  vfprintf(stderr, fmt, vargs);
  if (fmt[strlen(fmt) - 1] != '\n') {
    putc('\n', stderr);
  }
  va_end(vargs);
}
