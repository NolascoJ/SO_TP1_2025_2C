#ifndef MASTER_H
#define MASTER_H

#include "../utils/game_state.h"
#include "../utils/game_sync.h"
#include "../utils/game_config.h"

void init_board(game_state_t* game_state_ptr);

void create_players(int rfd[], unsigned int player_count, unsigned int width, unsigned int height, game_config_t config, game_state_t* game_state_ptr);

void init_game_state(game_config_t config, game_state_t* game_state_ptr);

void init_players(game_config_t config, game_state_t* game_state_ptr);

void init_game_sync(game_sync_t* sync_ptr, unsigned int player_count);

void destroy_game_sync(game_sync_t* sync_ptr, unsigned int player_count);

void close_player_pipes(int rfd[], unsigned int player_count);

void wait_for_children(game_state_t* game_state_ptr);

int process_player_move(game_state_t* game_state_ptr, unsigned int player_idx, char move);

void handle_player_input(int rfd[], unsigned int player_count, fd_set* readfds,
                        game_state_t* game_state_ptr, game_sync_t* game_sync_ptr,
                        game_config_t config, unsigned int* remaining_players, unsigned int start);

void handle_single_player(int player_idx, int fd, game_state_t* game_state_ptr, 
                         game_sync_t* game_sync_ptr, game_config_t config,
                         int rfd[], unsigned int* remaining_players);

int wait_for_player_activity(int max_fd, fd_set* readfds, unsigned int timeout);

int setup_fd_set(fd_set* readfds, int rfd[], unsigned int player_count);

#endif
