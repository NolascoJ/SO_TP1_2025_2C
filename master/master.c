// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com 

#define _GNU_SOURCE

#include "master.h"
#include "setup.h"

extern char **environ;

int main(int argc, char *argv[]) {

    game_config_t config = parse_arguments(argc, argv);

    int game_state_fd;
    int game_sync_fd;
    game_state_t* game_state_ptr;
    game_sync_t* game_sync_ptr;

    const size_t shm_state_size = sizeof(game_state_t) + ((size_t)config.width * (size_t)config.height * sizeof(int));
    const size_t shm_sync_size = sizeof(game_sync_t);

    game_state_ptr = (game_state_t*)shm_init("/game_state", shm_state_size, &game_state_fd, O_RDWR);
    game_sync_ptr = (game_sync_t*)shm_init("/game_sync", shm_sync_size, &game_sync_fd, O_RDWR);

    initialize_game(&config, game_state_ptr, game_sync_ptr);

    if (config.view_path != NULL) {
        create_view_process(&config, game_state_fd, game_sync_fd);
    }

    int rfd[config.player_count];
    create_players(rfd, &config, game_state_ptr);

    unsigned int remaining_players = config.player_count;
    time_t last_valid_move_time = time(NULL);

    // Priority algorithm for select() batch processing: when multiple players are ready
    // in the same select() call, prioritize players who didn't play last turn first,
    // then players who played last turn, both following round-robin order

    char player_activity1[config.player_count];
    char player_activity2[config.player_count];
    for (unsigned int i = 0; i < config.player_count; i++) {
        player_activity1[i] = 0;
        player_activity2[i] = 0;
    }
    

    char *played_last_turn = player_activity1;
    char *played_this_turn = player_activity2;

    unsigned int round_robin_start_index = 0;


    while(1){
        time_t now = time(NULL);
        if ((unsigned int)(now - last_valid_move_time) >= config.timeout || remaining_players == 0) {
            set_game_over(game_sync_ptr, game_state_ptr);
            break;
        }
        fd_set readfds;
        int max_fd = init_fd_set(&readfds, rfd, config.player_count);

        int ready = wait_for_fds(max_fd, &readfds, config.timeout);
        if (ready <= 0) { 
            set_game_over(game_sync_ptr, game_state_ptr);
            break;
        }

        sem_wait(&game_sync_ptr->master_mutex);
        sem_wait(&game_sync_ptr->game_state_mutex);
        handle_player_inputs(rfd, &readfds, &config, game_state_ptr, game_sync_ptr, &remaining_players, &last_valid_move_time, played_last_turn, played_this_turn, round_robin_start_index);
        sem_post(&game_sync_ptr->game_state_mutex);
        sem_post(&game_sync_ptr->master_mutex);
        
        round_robin_start_index = (round_robin_start_index + 1) % config.player_count;
        
        // Swap buffers for the next turn
        char *temp = played_last_turn;
        played_last_turn = played_this_turn;
        played_this_turn = temp;
    }

    if (config.view_path != NULL) {
            sem_post(&game_sync_ptr->master_to_view);
            sem_wait(&game_sync_ptr->view_to_master);
    }else {
        // Print final scores when no view is present
        printf("\n=== GAME OVER ===\n");
        printf("Final Scores:\n");
        for (unsigned int i = 0; i < config.player_count; i++) {
            printf("Player %u: Score = %u, Valid Moves = %u, Invalid Moves = %u\n", 
                i, 
                game_state_ptr->players[i].score,
                game_state_ptr->players[i].valid_moves,
                game_state_ptr->players[i].invalid_moves);
        }
        printf("================\n");
    }

    close_player_pipes(rfd, config.player_count);

    // Unblock any remaining players that might be waiting on semaphores
    for (unsigned int i = 0; i < config.player_count; i++) {
        sem_post(&game_sync_ptr->player_semaphores[i]);
    }

    while(wait(NULL) > 0) {
        // Continue waiting until no more children
    }

    destroy_game_sync(game_sync_ptr, config.player_count);

    shm_close(game_state_ptr, shm_state_size, game_state_fd);
    shm_destroy("/game_state");
    shm_close(game_sync_ptr, shm_sync_size, game_sync_fd);
    shm_destroy("/game_sync");

    return 0;
}

void create_players(int rfd[], const game_config_t *config, game_state_t *game_state_ptr) {
    for (unsigned int i = 0; i < config->player_count; i++) {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid == 0) { // Child process
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);

            execute_process(config->player_path[i], config->width, config->height);
        } else { // Parent process
            close(pipe_fd[1]);
            rfd[i] = pipe_fd[0];
            game_state_ptr->players[i].pid = pid;
        }
    }
}

void destroy_game_sync(game_sync_t* sync_ptr, unsigned int player_count) {
    sem_destroy(&sync_ptr->master_to_view);
    sem_destroy(&sync_ptr->view_to_master);
    sem_destroy(&sync_ptr->master_mutex);
    sem_destroy(&sync_ptr->game_state_mutex);
    sem_destroy(&sync_ptr->readers_count_mutex);

    for (unsigned int i = 0; i < player_count && i < GAME_SYNC_MAX_PLAYERS; i++) {
        sem_destroy(&sync_ptr->player_semaphores[i]);
    }
}

void close_player_pipes(int rfd[], unsigned int player_count) {
    for (unsigned int i = 0; i < player_count; i++) {
        if (rfd[i] != -1) {
            close(rfd[i]);
            rfd[i] = -1;
        }
    }
}

int check_neighbors(game_state_t* game_state_ptr, int x, int y, int width, int height, const int dx[8], const int dy[8]) {
    for (int m = 0; m < 8; m++) {
        int sx = x + dx[m];
        int sy = y + dy[m];
        if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
            int sidx = sy * width + sx;
            if (game_state_ptr->board_data[sidx] > 0) {
                return 1; // Has valid neighbors
            }
        }
    }
    return 0; // No valid neighbors
}

void process_player_move(game_state_t* game_state_ptr, unsigned int player_idx, char move, unsigned int* remaining_players, int rfd[], time_t* last_valid_move_time) {

    if (move > 7 || move < 0) {
        game_state_ptr->players[player_idx].invalid_moves++;
        return; 
    }

    int width = game_state_ptr->width;
    int height = game_state_ptr->height;

    // Direction vectors for 8-directional movement (N, NE, E, SE, S, SW, W, NW)
    const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

    unsigned short curx = game_state_ptr->players[player_idx].x_coord;
    unsigned short cury = game_state_ptr->players[player_idx].y_coord;

    if (!check_neighbors(game_state_ptr, (int)curx, (int)cury, width, height, dx, dy)) {
        game_state_ptr->players[player_idx].is_blocked = true;
        (*remaining_players)--;
        close(rfd[player_idx]);
        rfd[player_idx] = -1;
        return;
    }

    int move_index = (int)move;
    int next_x = (int)curx + dx[move_index];
    int next_y = (int)cury + dy[move_index];

    // Check board boundaries
    if (next_x < 0 || next_x >= width || next_y < 0 || next_y >= height) {
        game_state_ptr->players[player_idx].invalid_moves++;
        return;
    }

    int idx = next_y * width + next_x;
    int cell_value = game_state_ptr->board_data[idx];

    if (cell_value <= 0) {
        game_state_ptr->players[player_idx].invalid_moves++;
        return;
    }

    // Mark the old cell as visited by this player
    int old_idx = cury * width + curx;
    game_state_ptr->board_data[old_idx] = -((int)player_idx);

    // Update player values for valid move
    game_state_ptr->players[player_idx].valid_moves++;
    game_state_ptr->players[player_idx].score += (unsigned int)cell_value;
    game_state_ptr->board_data[idx] = -((int)player_idx);
    game_state_ptr->players[player_idx].x_coord = (unsigned short)next_x;
    game_state_ptr->players[player_idx].y_coord = (unsigned short)next_y;    // Update last valid move timestamp
    if (last_valid_move_time != NULL) {
        *last_valid_move_time = time(NULL);
    }

    // Check if player has valid neighbors from new position
    if (!check_neighbors(game_state_ptr, next_x, next_y, width, height, dx, dy)) {
        game_state_ptr->players[player_idx].is_blocked = true;
        (*remaining_players)--;
        close(rfd[player_idx]);
        rfd[player_idx] = -1;
    }
}

static void player_order(unsigned int process_order[], unsigned int* process_count, char played_this_turn[], const game_config_t* config, const char played_last_turn[], const int rfd[], const fd_set* readfds, unsigned int round_robin_start_index) {
    for (unsigned int i = 0; i < config->player_count; i++) {
        played_this_turn[i] = 0;
    }

    *process_count = 0;
    for (unsigned int i = 0; i < config->player_count; i++) {
        unsigned int player_idx = (round_robin_start_index + i) % config->player_count;
        if (!played_last_turn[player_idx] && rfd[player_idx] != -1 && FD_ISSET(rfd[player_idx], readfds)) {
            process_order[(*process_count)++] = player_idx;
        }
    }

    for (unsigned int i = 0; i < config->player_count; i++) {
        unsigned int player_idx = (round_robin_start_index + i) % config->player_count;
        if (played_last_turn[player_idx] && rfd[player_idx] != -1 && FD_ISSET(rfd[player_idx], readfds)) {
            process_order[(*process_count)++] = player_idx;
        }
    }
}


void handle_player_inputs(int rfd[], fd_set* readfds, const game_config_t *config,
                          game_state_t* game_state_ptr, game_sync_t* game_sync_ptr,
                          unsigned int* remaining_players, time_t* last_valid_move_time, char played_last_turn[], char played_this_turn[], unsigned int round_robin_start_index) {

    unsigned int process_order[config->player_count];
    unsigned int process_count = 0;
    player_order(process_order, &process_count, played_this_turn, config, played_last_turn, rfd, readfds, round_robin_start_index);


    for (unsigned int k = 0; k < process_count; k++) {
        unsigned int i = process_order[k];

        char c;
        ssize_t bytes_read = read(rfd[i], &c, 1);

        if (bytes_read > 0) {
            played_this_turn[i] = 1; 
            process_player_move(game_state_ptr, i, c, remaining_players, rfd, last_valid_move_time);

            if (config->view_path != NULL) {
                sem_post(&game_sync_ptr->master_to_view);
                sem_wait(&game_sync_ptr->view_to_master);
                usleep(config->delay * 1000);
            }
            sem_post(&game_sync_ptr->player_semaphores[i]);
        } else {
            if (!game_state_ptr->players[i].is_blocked) {
                (*remaining_players)--;
            }
            close(rfd[i]); 
            rfd[i] = -1;
        }
    }

}
void create_view_process(const game_config_t *config, int game_state_fd, int game_sync_fd) {
    pid_t pid = fork();
    if (pid == 0) { // Child process
 
        close(game_state_fd);
        close(game_sync_fd);
        
        // Execute the view process
        execute_process(config->view_path, config->width, config->height);
    }
}

void execute_process(const char *path, unsigned int width, unsigned int height) {
    char width_str[16];
    char height_str[16];
    snprintf(width_str, sizeof(width_str), "%u", width);
    snprintf(height_str, sizeof(height_str), "%u", height);
    
    execve(path, (char*[]){(char*)path, width_str, height_str, NULL}, environ);
    perror("execve");
    exit(EXIT_FAILURE);
}

void set_game_over(game_sync_t* game_sync_ptr, game_state_t* game_state_ptr) {
    sem_wait(&game_sync_ptr->master_mutex);
    sem_wait(&game_sync_ptr->game_state_mutex);
    
    game_state_ptr->game_over = true;

    
    sem_post(&game_sync_ptr->game_state_mutex);
    sem_post(&game_sync_ptr->master_mutex);
}