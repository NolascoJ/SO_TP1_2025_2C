// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com 

#include "player_lib.h"
#include <fcntl.h>

// Implementaciones compartidas por todos los jugadores. No definir `getMove` aquí;
// cada jugador debe definir su propia versión en su archivo de jugador. 

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
    for (unsigned int i = 0; i < game_state_ptr->player_count; i++) {
        if (game_state_ptr->players[i].pid == pid) {
            return i;
        }
    }
    perror("I dont exist in the board");
    return 1;
}

void acquire_read_lock(game_sync_t* game_sync_ptr, int me) {
    sem_wait(&game_sync_ptr->player_semaphores[me]); // semáforo por jugador
    sem_wait(&game_sync_ptr->master_mutex); // mutex para evitar inanición del máster
    sem_post(&game_sync_ptr->master_mutex); // libero mutex
    sem_wait(&game_sync_ptr->readers_count_mutex); // mutex para la variable readers_count
    game_sync_ptr->readers_count++;
    if (game_sync_ptr->readers_count == 1) {
        sem_wait(&game_sync_ptr->game_state_mutex); // alguien está leyendo, bloquea al máster
    }
    sem_post(&game_sync_ptr->readers_count_mutex);
}

void release_read_lock(game_sync_t* game_sync_ptr) {
    sem_wait(&game_sync_ptr->readers_count_mutex);
    game_sync_ptr->readers_count--;
    if (game_sync_ptr->readers_count == 0) {
        sem_post(&game_sync_ptr->game_state_mutex); // ya no hay lectores
    }
    sem_post(&game_sync_ptr->readers_count_mutex);
}

void take_snapshot(game_state_t* game_state_ptr, player_t* playerList, game_state_t* state, int gameWidth, int gameHeight) {
    memcpy(playerList, game_state_ptr->players, sizeof(playerList[0]) * game_state_ptr->player_count);
    memcpy(state, game_state_ptr, sizeof(game_state_t) + sizeof(int) * gameWidth * gameHeight);
}

// Main compartido para todos los jugadores. Llama a la función getMove definida
// en cada implementación concreta del jugador (player.c). Se reutiliza la lógica
// de acceso a memoria compartida y sincronización ya presente en este archivo.
int main(int argc, char* argv[]) {

    if (argc < 3) {
        return 1;
    }
 
    int gameWidth = atoi(argv[1]);
    int gameHeight = atoi(argv[2]);
    int providedIndex = (argc >= 4) ? atoi(argv[3]) : -1;
    
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

    int me = (providedIndex >= 0) ? providedIndex : (int)getMe(game_state_ptr);
    player_t playerList[9];
    char state_buffer[sizeof(game_state_t) + gameWidth * gameHeight * sizeof(int)];
    game_state_t* state = (game_state_t*)state_buffer;

    while (true) {
        acquire_read_lock(game_sync_ptr, me);

        take_snapshot(game_state_ptr, playerList, state, gameWidth, gameHeight);

        // Check termination conditions AFTER acquiring the lock to avoid race conditions
        if (game_state_ptr->game_over || game_state_ptr->players[me].is_blocked) {
            release_read_lock(game_sync_ptr);
            break;
        }

        release_read_lock(game_sync_ptr);

        int move = getMove(playerList, state, me);
        write(STDOUT_FILENO, &move, 1);

    }

    cleanup_resources(game_state_ptr, game_sync_ptr, shm_state_size, shm_sync_size, game_state_fd, game_sync_fd);
    return 0;
}
