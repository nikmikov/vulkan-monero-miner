#include "cli_opts.h"

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

void print_usage(const char *name)
{
  fprintf(stdout, "Usage: %s --config <config.json>\n", name);
  exit(EXIT_FAILURE);
}

void parse_cli_opts(int argc, char **argv, struct cli_opts *opts)
{
  assert(opts->config_file == NULL);
  static struct option long_opts[] = {{"help", no_argument, NULL, 'h'},
                                      {"config", required_argument, NULL, 'c'},
                                      {NULL, 0, NULL, 0}};

  const char *short_opts = "hc:";

  int c;
  while ((c = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
    switch (c) {
    case -1: /* no more arguments */
    case 0:  /* long options toggles */
      break;
    case 'c':
      opts->config_file = optarg;
      break;
    case 'h':
      print_usage(argv[0]);
      exit(EXIT_FAILURE);
    default:
      fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
      fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (opts->config_file == NULL) {
    fprintf(stderr, "Required parameter --config not specified.\n");
    print_usage(argv[0]);
    exit(EXIT_FAILURE);
  }
}
