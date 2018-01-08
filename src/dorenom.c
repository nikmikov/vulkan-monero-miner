#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "cli_opts.h"
#include "config.h"
#include "console.h"
#include "logging.h"
#include "miner.h"


static const char *uv_handle_type_str[] = {
  "UV_UNKNOWN_HANDLE",
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
  "UV_HANDLE_TYPE_MAX"
};


void on_uv_walk(uv_handle_t* handle, void* arg)
{
  log_debug("Closing handle: %p, type: %s", handle, uv_handle_type_str[handle->type]);
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
  //uv_print_all_handles(uv_default_loop(), stdout);
  uv_stop(uv_default_loop());
}

int main(int argc, char **argv) {

  // parse options
  struct cli_opts cli_opts = {0};
  parse_cli_opts(argc, argv, &cli_opts);


  // TODO: init console

  // read config
  struct config* cfg = NULL;
  if(!config_from_file(cli_opts.config_file, &cfg)) {
    log_error("Error when reading config.");
    exit(1);
  }

  assert(cfg->size > 0);

  int exit_code = 0;
  miner_handle *miners = calloc(cfg->size, sizeof(miner_handle));
  for(size_t i = 0; i < cfg->size; ++i) {
    miners[i] = miner_init(&cfg->miners[i]);
    if(miners[i] == NULL) {
      exit_code = 1;
      goto SHUTDOWN;
    }
  }

  uv_loop_t *loop = uv_default_loop();
  // install sigint handler
  uv_signal_t sigint;
  uv_signal_init(loop, &sigint);
  uv_signal_start(&sigint, on_sigint_received, SIGINT);
  uv_unref((uv_handle_t*)&sigint);


  // instantiate pool handlers
  /*pool_connection_handle h = NULL;
  if(!pool_connection_init(&cfg->miners[0].pool_list, &h)) {
    log_error("Connection init failed");
    exit(1);
  }

  pool_connection_connect(h);*/

  for(size_t i = 0; i < cfg->size; ++i) {
    miner_start(miners[i]);
  }

  log_debug("Starting event loop");
  uv_run(loop, UV_RUN_DEFAULT);
 SHUTDOWN:
  log_debug("Shutting down.");
  for(size_t i = 0; i < cfg->size; ++i) {
    if(miners[i]) {
      miner_stop(miners[i]);
      miner_free(&miners[i]);
    }
  }
  free(miners);
  config_free(cfg);

  return exit_code;
}
