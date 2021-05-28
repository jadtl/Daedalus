#pragma once

#include "core/application.h"
#include "core/logger.h"
#include "core/ddls_memory.h"
#include "game_types.h"

extern b8 create_game(game* out_game);

int main(void) {
  initialize_memory();

  game game_inst;
  if (!create_game(&game_inst)) {
    DDLS_FATAL("Could not create game!");
    return -1;
  }

  if (!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.on_resize) {
    DDLS_FATAL("The game's function pointers must be assigned!");
    return -2;
  }

  if (!application_create(&game_inst)) {
    DDLS_INFO("Application failed to create.");
    return 1;
  }

  if (!application_run()) {
    DDLS_INFO("Application did not shutdown gracefully.");
    return 2;
  }

  shutdown_memory();

  return 0;
}