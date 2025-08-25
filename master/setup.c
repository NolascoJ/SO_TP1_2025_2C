#include "setup.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include "master.h"

// Private functions - only visible within this file
static void init_board(game_state_t* game_state_ptr) {
    for (unsigned int i = 0; i < game_state_ptr->height * game_state_ptr->width; i++) {
        game_state_ptr->board_data[i] = rand() % 9 + 1;
    }
}

static void init_game_state(const game_config_t* config, game_state_t *game_state_ptr) {
    game_state_ptr->width = config->width;
    game_state_ptr->height = config->height;
    game_state_ptr->player_count = config->player_count;
    game_state_ptr->game_over = false;
    init_board(game_state_ptr);
}

static void init_players(const game_config_t* config, game_state_t* game_state_ptr) {
    for (unsigned int i = 0; i < config->player_count; i++) {
        const char* player_name = strrchr(config->player_path[i], '/');
        player_name = player_name ? player_name + 1 : config->player_path[i];
        
        strncpy(game_state_ptr->players[i].name, player_name, sizeof(game_state_ptr->players[i].name) - 1);
        game_state_ptr->players[i].name[sizeof(game_state_ptr->players[i].name) - 1] = '\0';
        
        game_state_ptr->players[i].score = 0;
        game_state_ptr->players[i].valid_moves = 0;
        game_state_ptr->players[i].invalid_moves = 0;
        game_state_ptr->players[i].x_coord = rand() % config->width;
        game_state_ptr->players[i].y_coord = rand() % config->height;
        game_state_ptr->players[i].pid = 0;
        game_state_ptr->players[i].is_blocked = false;
    }
}

static void init_game_sync(game_sync_t* sync_ptr, unsigned int player_count) {
    sem_init(&sync_ptr->master_to_view, 1, 0);
    sem_init(&sync_ptr->view_to_master, 1, 0);
    sem_init(&sync_ptr->master_mutex, 1, 1);
    sem_init(&sync_ptr->game_state_mutex, 1, 1);
    sem_init(&sync_ptr->readers_count_mutex, 1, 1);
    sync_ptr->readers_count = 0;
    
    for (unsigned int i = 0; i < player_count && i < GAME_SYNC_MAX_PLAYERS; i++) {
        sem_init(&sync_ptr->player_semaphores[i], 1, 1);
    }
}

// Public function
void initialize_game(const game_config_t *config, game_state_t *game_state, game_sync_t *game_sync) {
    srand(config->seed);
    init_game_state(config, game_state);
    init_players(config, game_state);
    init_game_sync(game_sync, config->player_count);
}
