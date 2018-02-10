#pragma once

#include <stdbool.h>
#include <unistd.h>

struct cli_opts {
  /** Path to config file */
  char *config_file;
  bool is_benchmark;
};

void parse_cli_opts(int argc, char **argv, struct cli_opts *opts);
