#include <stdio.h>

#include <uv.h>

#include "cli_opts.h"
#include "config.h"
#include "console.h"
#include "logging.h"
#include "pool_connection.h"


struct status {
  uv_timer_t timer;
};


void update_status_cb(uv_timer_t* handle) {
  printf("Status:\n");
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
  }

  // instantiate pool handlers

  // start main loop
  struct status st;
  uv_loop_t *loop = uv_default_loop();

  uv_timer_init(loop, &st.timer);
  uv_timer_start(&st.timer, update_status_cb, 0, 2000);

  printf("Now quitting.\n");
  //uv_run(loop, UV_RUN_DEFAULT);


  //uv_loop_close(loop);

  config_free(cfg);

  return 0;
}
