// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com 

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include "master.h"
#include "setup.h"
#include "../utils/game_state.h"
#include "../utils/game_sync.h"
#include "../shared_memory/shm.h"
#include "../utils/game_config.h"
#include "../utils/socket_utils.h"

extern char **environ;
static time_t last_valid_move_time;

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
    last_valid_move_time = time(NULL);

    while (!game_state_ptr->game_over) {
        // Timeout based on time since last valid move
        time_t now = time(NULL);
        if ((unsigned int)(now - last_valid_move_time) >= config.timeout) {
            printf("Timeout reached. No valid moves in %u seconds. Ending game.\n", config.timeout);
            game_state_ptr->game_over = true;
            // Wake view for a final update so it can exit
            if (config.view_path != NULL) {
                sem_post(&game_sync_ptr->master_to_view);
                sem_wait(&game_sync_ptr->view_to_master);
            }
            break;
        }
        fd_set readfds;
        int max_fd = init_fd_set(&readfds, rfd, config.player_count);

        int ready = wait_for_fds(max_fd, &readfds, config.timeout);
        if (ready == 0) { // Timeout
            printf("Timeout reached. Ending game due to inactivity.\n");
            game_state_ptr->game_over = true;
            // Wake view for a final update so it can exit
            if (config.view_path != NULL) {
                sem_post(&game_sync_ptr->master_to_view);
                sem_wait(&game_sync_ptr->view_to_master);
            }
            break;
        }
        if (ready < 0) { // Error
            game_state_ptr->game_over = true;
            // Wake view for a final update so it can exit
            if (config.view_path != NULL) {
                sem_post(&game_sync_ptr->master_to_view);
                sem_wait(&game_sync_ptr->view_to_master);
            }
            break;
        }

        sem_wait(&game_sync_ptr->master_mutex);
        handle_player_inputs(rfd, &readfds, &config, game_state_ptr, game_sync_ptr, &remaining_players);
        sem_post(&game_sync_ptr->master_mutex);

        if (remaining_players == 0) {
            sem_wait(&game_sync_ptr->game_state_mutex);
            printf("DEBUG: All players finished! Setting game_over=true\n");
            game_state_ptr->game_over = true;
            sem_post(&game_sync_ptr->game_state_mutex);

            // Signal view to update one final time
            if (config.view_path != NULL) {
                sem_post(&game_sync_ptr->master_to_view);
                sem_wait(&game_sync_ptr->view_to_master);
            }
        }
    }

    close_player_pipes(rfd, config.player_count);

    // Unblock any remaining players that might be waiting on semaphores
    for (unsigned int i = 0; i < config.player_count; i++) {
        sem_post(&game_sync_ptr->player_semaphores[i]);
    }

    // Wait for any remaining child processes to terminate (view process, etc.)
    while(wait(NULL) > 0) {
        // Continue waiting until no more children
    }

    destroy_game_sync(game_sync_ptr, config.player_count);

    shm_close(game_state_ptr, shm_state_size, game_state_fd);
    shm_destroy("/game_state");
    shm_close(game_sync_ptr, shm_sync_size, game_sync_fd);
    shm_destroy("/game_sync");

    printf("JOYA\n");
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

            char width_str[16];
            char height_str[16];
            snprintf(width_str, sizeof(width_str), "%u", config->width);
            snprintf(height_str, sizeof(height_str), "%u", config->height);
            execve(config->player_path[i], (char*[]){(char*)config->player_path[i], width_str, height_str, NULL}, environ);
            
            perror("execve");
            exit(EXIT_FAILURE);
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

int process_player_move(game_state_t* game_state_ptr, unsigned int player_idx, char move, unsigned int* remaining_players, int rfd[]) {
    unsigned char code = (unsigned char)move;

    int width = game_state_ptr->width;
    int height = game_state_ptr->height;

    int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
    int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

    unsigned short curx = game_state_ptr->players[player_idx].x_coord;
    unsigned short cury = game_state_ptr->players[player_idx].y_coord;

    // If the player has no valid neighbors from the current position, mark as blocked
    int valid_neighbors_from_current = 0;
    for (int m = 0; m < 8; m++) {
        int sx = (int)curx + dx[m];
        int sy = (int)cury + dy[m];
        if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
            int sidx = sy * width + sx;
            if (game_state_ptr->board_data[sidx] > 0) {
                valid_neighbors_from_current++;
                break;
            }
        }
    }
    if (valid_neighbors_from_current == 0) {
        game_state_ptr->players[player_idx].is_blocked = true;
        (*remaining_players)--;
        close(rfd[player_idx]);
        rfd[player_idx] = -1;
        return 1; // state changed (player blocked)
    }

    if (code > 7 ) {
        game_state_ptr->players[player_idx].invalid_moves++;
        return 1; // state changed (invalid move counter)
    }

    int nx = (int)curx + dx[code];
    int ny = (int)cury + dy[code];
    // Límites del tablero
    if (nx < 0 || nx >= width || ny < 0 || ny >= height) {
        game_state_ptr->players[player_idx].invalid_moves++;
        return 1; // state changed (invalid move counter)
    }

    int idx = ny * width + nx;
    int cell_value = game_state_ptr->board_data[idx];

    // Celda válida
    if (cell_value <= 0) {
        game_state_ptr->players[player_idx].invalid_moves++;
        return 1; // state changed (invalid move counter)
    }

    // Actualizar valores del jugador
    game_state_ptr->players[player_idx].valid_moves++;
    game_state_ptr->players[player_idx].score += (unsigned int)cell_value;
    game_state_ptr->board_data[idx] = -((int)player_idx); 
    game_state_ptr->players[player_idx].x_coord = (unsigned short)nx;
    game_state_ptr->players[player_idx].y_coord = (unsigned short)ny;

    // Update last valid move timestamp
    last_valid_move_time = time(NULL);

    //esto lo hice static pero se podria agregar al contrato de la funcion

    // Casillas vecinas válidas
    int valid_neighbors = 0;
    for (int m = 0; m < 8; m++) {
        int sx = (int)nx + dx[m];
        int sy = (int)ny + dy[m];

        if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
            int sidx = sy * width + sx;
            if (game_state_ptr->board_data[sidx] > 0) {
                valid_neighbors++;
                break; // al menos un movimiento válido
            }
        }
    }
    if (valid_neighbors == 0) {
        game_state_ptr->players[player_idx].is_blocked = true;
        (*remaining_players)--;
        close(rfd[player_idx]);
        rfd[player_idx] = -1;
    }

    return 1;
}

void handle_player_inputs(int rfd[], fd_set* readfds, const game_config_t *config,
                          game_state_t* game_state_ptr, game_sync_t* game_sync_ptr,
                          unsigned int* remaining_players) {
    for (unsigned int i = 0; i < config->player_count; i++) {
        if (rfd[i] != -1 && FD_ISSET(rfd[i], readfds)) {
            char c;
            ssize_t bytes_read = read(rfd[i], &c, 1);

            if (bytes_read > 0) {
                sem_wait(&game_sync_ptr->game_state_mutex);
                int cambios = process_player_move(game_state_ptr, i, c, remaining_players, rfd);
                sem_post(&game_sync_ptr->game_state_mutex);

                if (config->view_path != NULL && cambios) {
                    sem_post(&game_sync_ptr->master_to_view);
                    sem_wait(&game_sync_ptr->view_to_master);
                    usleep(config->delay * 1000);
                }
                sem_post(&game_sync_ptr->player_semaphores[i]);
            } else {
                // Only decrement if not already decremented in process_player_move
                if (!game_state_ptr->players[i].is_blocked) {
                    (*remaining_players)--;
                }
                close(rfd[i]);
                rfd[i] = -1;

                if (bytes_read < 0) {
                    perror("read from player");
                }
            }
        }
    }
}

void create_view_process(const game_config_t *config, int game_state_fd, int game_sync_fd) {
    pid_t pid = fork();
    if (pid == 0) { // Child process
 
        close(game_state_fd);
        close(game_sync_fd);
        
        // Execute the view process
        char width_str[16];
        char height_str[16];
        snprintf(width_str, sizeof(width_str), "%u", config->width);
        snprintf(height_str, sizeof(height_str), "%u", config->height);
        execve(config->view_path, (char*[]){(char*)config->view_path, width_str, height_str, NULL}, environ);
        perror("execve");
        exit(EXIT_FAILURE);
    }
}
