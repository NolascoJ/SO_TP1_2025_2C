#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#include "../utils/game_state.h"
#include "../utils/game_sync.h"
#include "../shared_memory/shm.h"
#include "player.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        return 1;
    }
    
    int gameWidth = atoi(argv[1]);
    int gameHeight = atoi(argv[2]);
    
    int game_state_fd, game_sync_fd;
    game_state_t* game_state_ptr;
    game_sync_t* game_sync_ptr;
    
    const size_t shm_state_size = sizeof(game_state_t) + (gameWidth * gameHeight * sizeof(int));
    const size_t shm_sync_size = sizeof(game_sync_t);

    game_state_ptr = shm_open_and_map("/game_state", shm_state_size, &game_state_fd, O_RDONLY);
    if (game_state_ptr == NULL) {
        return 1;
    }
    
    game_sync_ptr = shm_open_and_map("/game_sync", shm_sync_size, &game_sync_fd, O_RDWR);
    if (game_sync_ptr == NULL) {
        cleanup_resources(game_state_ptr, NULL, shm_state_size, 0, game_state_fd, 0);
        return 1;
    }

    int me = getMe(game_state_ptr);
    player_t playerList[9];
    char state_buffer[sizeof(game_state_t) + gameWidth * gameHeight * sizeof(int)];
    game_state_t* state = (game_state_t*)state_buffer;

    while (!game_state_ptr->game_over && !game_state_ptr->players[me].is_blocked) {
        
        acquire_read_lock(game_sync_ptr, me);

        take_snapshot(game_state_ptr, playerList, state, gameWidth, gameHeight);

        release_read_lock(game_sync_ptr);

        int move = getMove(playerList, state, me);

        printf("%d", move);
        fflush(stdout);

    }

    cleanup_resources(game_state_ptr, game_sync_ptr, shm_state_size, shm_sync_size, game_state_fd, game_sync_fd);
    return 0;
}

void cleanup_resources(game_state_t* game_state_ptr, game_sync_t* game_sync_ptr, 
                      size_t state_size, size_t sync_size, int state_fd, int sync_fd) {
    if (game_state_ptr) {
        shm_close(game_state_ptr, state_size, state_fd);
    }
    if (game_sync_ptr) {
        shm_close(game_sync_ptr, sync_size, sync_fd);
    }
}

unsigned int getMe(game_state_t* game_state_ptr) {
    pid_t pid = getpid();
    for (int i = 0; i < game_state_ptr->player_count; i++) {
        if (game_state_ptr->players[i].pid == pid) {
            return i;
        }
    }
    errExit("I dont exist in the board");
}

void acquire_read_lock(game_sync_t* game_sync_ptr, int me) {
    sem_wait(&game_sync_ptr->G[me]);
    sem_wait(&game_sync_ptr->C);
    sem_post(&game_sync_ptr->C);
    sem_wait(&game_sync_ptr->E);
    game_sync_ptr->F++;
    if (game_sync_ptr->F == 1) {
        sem_wait(&game_sync_ptr->D);
    }
    sem_post(&game_sync_ptr->E);
}

void release_read_lock(game_sync_t* game_sync_ptr) {
    sem_wait(&game_sync_ptr->E);
    game_sync_ptr->F--;
    if (game_sync_ptr->F == 0) {
        sem_post(&game_sync_ptr->D);
    }
    sem_post(&game_sync_ptr->E);
}

void take_snapshot(game_state_t* game_state_ptr, player_t* playerList, game_state_t* state, int gameWidth, int gameHeight) {
    memcpy(playerList, game_state_ptr->players, sizeof(playerList[0]) * game_state_ptr->player_count);
    memcpy(state, game_state_ptr, sizeof(game_state_t) + sizeof(int) * gameWidth * gameHeight);
}

int getMove(player_t* playerList, game_state_t* state, int me) {
    return 2;
}
