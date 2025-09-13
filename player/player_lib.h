#ifndef PLAYER_LIB_H
#define PLAYER_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#include "../common/game_state.h"
#include "../common/game_sync.h"
#include "../shared_memory/shm.h"

void cleanup_resources(game_state_t* game_state_ptr, game_sync_t* game_sync_ptr,
                       size_t state_size, size_t sync_size, int state_fd, int sync_fd);

int getMe(game_state_t* game_state_ptr, game_sync_t* game_sync_ptr);

void acquire_read_lock(game_sync_t* game_sync_ptr, int me);

void release_read_lock(game_sync_t* game_sync_ptr);

void take_snapshot(game_state_t* game_state_ptr, player_t* playerList, game_state_t* state, int gameWidth, int gameHeight);

int getMove(player_t* playerList, game_state_t* state, int me);

#endif 
