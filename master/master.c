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

int main(int argc, char *argv[]) {
    printf("Proceso Máster iniciado.\n");

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

    int rfd[config.player_count];
    create_players(rfd, &config, game_state_ptr);

    unsigned int remaining_players = config.player_count;

    while (!game_state_ptr->game_over) {
        fd_set readfds;
        int max_fd = init_fd_set(&readfds, rfd, config.player_count);

        int ready = wait_for_fds(max_fd, &readfds, config.timeout);
        if (ready == 0) { // Timeout
            printf("Timeout reached. Ending game due to inactivity.\n");
            game_state_ptr->game_over = true;
            break;
        }
        if (ready < 0) { // Error
            game_state_ptr->game_over = true;
            break;
        }

        sem_wait(&game_sync_ptr->master_mutex);
        handle_player_inputs(rfd, &readfds, &config, game_state_ptr, game_sync_ptr, &remaining_players);
        sem_post(&game_sync_ptr->master_mutex);

        if (remaining_players == 0) {
            game_state_ptr->game_over = true;
        }
    }

    close_player_pipes(rfd, config.player_count);
    
    // Wait for all child processes to terminate
    while(wait(NULL) > 0);

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

int process_player_move(game_state_t* game_state_ptr, unsigned int player_idx, char move) {
    game_state_ptr->players[player_idx].valid_moves++;
    printf("Jugador %u se movió a %c\n", player_idx, move);
    
    if (game_state_ptr->players[player_idx].valid_moves >= 50) {
        game_state_ptr->players[player_idx].is_blocked = true;
        printf("Jugador %u bloqueado después de %u movimientos\n", player_idx, game_state_ptr->players[player_idx].valid_moves);
    }
    return 0;
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
                int cambios = process_player_move(game_state_ptr, i, c);
                sem_post(&game_sync_ptr->game_state_mutex);

                if (cambios) {
                    sem_post(&game_sync_ptr->master_to_view);
                    sem_wait(&game_sync_ptr->view_to_master);
                    usleep(config->delay * 1000);
                }
                sem_post(&game_sync_ptr->player_semaphores[i]);
            } else {
                close(rfd[i]);
                rfd[i] = -1;
                (*remaining_players)--;
                if (bytes_read < 0) {
                    perror("read from player");
                }
            }
        }
    }
}
