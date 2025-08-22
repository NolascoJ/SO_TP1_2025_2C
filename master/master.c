#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>

#include "../utils/game_state.h"
#include "../shared_memory/shm.h"

int main() {
    printf("Proceso Máster iniciado.\n");
    
    int game_state_fd;
    game_state_t* game_state_ptr;

    // El tamaño del tablero se pasa aparte porque es variable -> después hacerlo según parámetros
    const size_t shm_state_size = sizeof(game_state_t) + (10 * 10 * sizeof(int)); 

    game_state_ptr = (game_state_t*)shm_init("/game_state", shm_state_size, &game_state_fd, O_RDWR);
    if (game_state_ptr == NULL) {
        perror("Fallo al inicializar la memoria de estado.");
        return 1;
    }
    printf("Máster: Memoria /game_state creada y mapeada.\n");
    game_state_ptr->player_count = 2;

// fork = 0 ->child
// fork > 0 -> parent
// fork < 0 -> error

    if (fork() == 0) {
        char* argv[] = {"view", NULL};
        execve("./bin/view", argv, NULL);
        perror("execve");
        exit(1);
    }else{
        waitpid(-1, NULL, 0);
    }

    // Create player process
    if (fork() == 0) {
        char* argv[] = {"player", NULL};
        execve("./bin/player", argv, NULL);
        perror("execve");
        exit(1);
    }else{
        waitpid(-1, NULL, 0);
    }
    
    shm_close(game_state_ptr, shm_state_size, game_state_fd);
    shm_destroy("/game_state");
    
    printf("Máster: Finalizado. Recursos liberados.\n");
    return 0;
}