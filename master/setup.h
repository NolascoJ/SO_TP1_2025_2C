#ifndef SETUP_H
#define SETUP_H

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include "../utils/game_config.h"
#include "../utils/game_state.h"
#include "../utils/game_sync.h"

void initialize_game(const game_config_t *config, game_state_t *game_state, game_sync_t *game_sync);

#endif // SETUP_H
