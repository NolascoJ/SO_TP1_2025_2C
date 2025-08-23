#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include "../utils/game_state.h"
#include "../shared_memory/shm.h"

extern char **environ;
void init_board(game_state_t* game_state_ptr);

int main(int argc, char *argv[]) {
    printf("Proceso Máster iniciado.\n");
    srand(time(NULL));  // Initialize random seed
    
    int game_state_fd;
    game_state_t* game_state_ptr;

    // Valores por defecto y parseo de argumentos
    unsigned int player_count = 2;
    unsigned int width = 10;
    unsigned int height = 10;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            player_count = (unsigned int)atoi(argv[++i]);
        } else if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            width = (unsigned int)atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            height = (unsigned int)atoi(argv[++i]);
        }
    }

    // El tamaño del tablero se pasa aparte porque es variable -> después hacerlo según parámetros
    const size_t shm_state_size = sizeof(game_state_t) + ((size_t)width * (size_t)height * sizeof(int)); 

    game_state_ptr = (game_state_t*)shm_init("/game_state", shm_state_size, &game_state_fd, O_RDWR);
    
    printf("Máster: Memoria /game_state creada y mapeada.\n");
    game_state_ptr->player_count = player_count;
    game_state_ptr->width = (unsigned short)width;
    game_state_ptr->height = (unsigned short)height;
    

init_board(game_state_ptr);

// fork = 0 ->child
// fork > 0 -> parent
// fork < 0 -> error

    if (fork() == 0) {
        char* argv_view[] = {"view", NULL};
        setenv("TERM", "xterm-256color", 1);
        execve("./bin/view", argv_view, environ);
        perror("execve");
        exit(1);
    }else{
        waitpid(-1, NULL, 0);
    }

    // // Create player process
    // if (fork() == 0) {
    //     char* argv[] = {"player", NULL};
    //     execve("./bin/player", argv, NULL);
    //     perror("execve");
    //     exit(1);
    // }else{
    //     waitpid(-1, NULL, 0);
    // }
    
    shm_close(game_state_ptr, shm_state_size, game_state_fd);
    shm_destroy("/game_state");
    
    printf("Máster: Finalizado. Recursos liberados.\n");
    return 0;
}


void init_board(game_state_t* game_state_ptr) {
    for (unsigned int i = 0; i < game_state_ptr->height * game_state_ptr->width; i++) {
        game_state_ptr->board_data[i] = rand() % 10;  // Random values 0-9
    }
}