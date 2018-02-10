#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <uv.h>

#include "cli_opts.h"
#include "config.h"
#include "console.h"
#include "foreman.h"
#include "logging.h"

static const char *uv_handle_type_str[] = {"UV_UNKNOWN_HANDLE",
                                           "UV_ASYNC",
                                           "UV_CHECK",
                                           "UV_FS_EVENT",
                                           "UV_FS_POLL",
                                           "UV_HANDLE",
                                           "UV_IDLE",
                                           "UV_NAMED_PIPE",
                                           "UV_POLL",
                                           "UV_PREPARE",
                                           "UV_PROCESS",
                                           "UV_STREAM",
                                           "UV_TCP",
                                           "UV_TIMER",
                                           "UV_TTY",
                                           "UV_UDP",
                                           "UV_SIGNAL",
                                           "UV_FILE",
                                           "UV_HANDLE_TYPE_MAX"};

void on_uv_walk(uv_handle_t *handle, void *arg)
{
  log_debug("Closing handle: %p, type: %s", handle,
            uv_handle_type_str[handle->type]);
  uv_close(handle, NULL);
}

void on_sigint_received(uv_signal_t *handle, int signum)
{
  log_warn("SIGINT received. Gracefully shutting down");
  int result = uv_loop_close(handle->loop);
  if (result == UV_EBUSY) {
    log_debug("Closing active event handles");
    uv_walk(handle->loop, on_uv_walk, NULL);
  }
  // uv_print_all_handles(uv_default_loop(), stdout);
  uv_stop(uv_default_loop());
}

int main(int argc, char **argv)
{
  srand(time(NULL));
  // parse options
  struct cli_opts cli_opts = {0};
  parse_cli_opts(argc, argv, &cli_opts);

  // TODO: init console

  // read config
  struct config *cfg = config_from_file(cli_opts.config_file);
  if (cfg == NULL) {
    log_error("Error when reading config.");
    exit(1);
  }
  size_t cfg_size = 0;
  for (struct config *p = cfg; p != NULL; ++cfg_size) {
    p = p->next;
  }
  assert(cfg_size > 0);

  int exit_code = 0;
  foreman_handle *foremans = calloc(cfg_size, sizeof(foreman_handle));

  struct config *p = cfg;
  for (size_t i = 0; i < cfg_size; ++i, p = p->next) {
    foremans[i] = foreman_new(p, cli_opts.is_benchmark);
    if (foremans[i] == NULL) {
      exit_code = 1;
      goto SHUTDOWN;
    }
  }

  uv_loop_t *loop = uv_default_loop();
  // install sigint handler
  uv_signal_t sigint;
  uv_signal_init(loop, &sigint);
  uv_signal_start(&sigint, on_sigint_received, SIGINT);
  uv_unref((uv_handle_t *)&sigint);

  // instantiate pool handlers
  /*pool_connection_handle h = NULL;
  if(!pool_connection_init(&cfg->foremans[0].pool_list, &h)) {
    log_error("Connection init failed");
    exit(1);
  }

  pool_connection_connect(h);*/

  for (size_t i = 0; i < cfg_size; ++i) {
    foreman_start(foremans[i]);
  }

  log_debug("Starting event loop");
  uv_run(loop, UV_RUN_DEFAULT);
SHUTDOWN:
  log_debug("Shutting down.");
  for (size_t i = 0; i < cfg_size; ++i) {
    if (foremans[i]) {
      foreman_stop(foremans[i]);
      foreman_free(&foremans[i]);
    }
  }
  free(foremans);
  if (cfg != NULL) {
    cfg->free(cfg);
  }

  return exit_code;
}
