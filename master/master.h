#ifndef MASTER_H
#define MASTER_H

#include <sys/select.h>
#include "../utils/game_config.h"
#include "../utils/game_state.h"
#include "../utils/game_sync.h"


void create_players(int rfd[], const game_config_t *config, game_state_t *game_state_ptr);
void create_view_process(const game_config_t *config, int game_state_fd, int game_sync_fd);
void destroy_game_sync(game_sync_t* sync_ptr, unsigned int player_count);
void close_player_pipes(int rfd[], unsigned int player_count);
int process_player_move(game_state_t* game_state_ptr, unsigned int player_idx, char move, unsigned int* remaining_players, int rdf[]);
void handle_player_inputs(int rfd[], fd_set* readfds, const game_config_t *config, 
                          game_state_t* game_state_ptr, game_sync_t* game_sync_ptr, 
                          unsigned int* remaining_players);

#endif // MASTER_H
