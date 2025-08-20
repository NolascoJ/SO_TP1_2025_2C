#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "../utils/game_state.h"
#include "../shared_memory/shm.h"


int main() {
    printf("Proceso Vista iniciado.\n");

    int game_state_fd;
    game_state_t* game_state_ptr;

    // El tamaño de la matriz se pasa aparte pq es variable
    const size_t shm_state_size = sizeof(game_state_t) + (10 * 10 * sizeof(int)); 

    game_state_ptr = (game_state_t*)shm_open_and_map("/game_state", shm_state_size, &game_state_fd, O_RDONLY);
    if (game_state_ptr == NULL) {
        perror("Fallo al abrir la memoria de estado.");
        return 1;
    }
    printf("Vista: Conectada a /game_state.\n");

    printf("Cantidad de jugadores: %d\n", game_state_ptr->player_count);

    shm_close(game_state_ptr, shm_state_size, game_state_fd);

    printf("Vista: Finalizada.\n");
    return 0;
}