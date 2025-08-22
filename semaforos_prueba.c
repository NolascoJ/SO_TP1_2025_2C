#include "shared_memory/shm.h"
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

typedef struct {
    sem_t A; /* El máster le indica a la vista que hay cambios por imprimir */
    sem_t B; /* La vista le indica al máster que terminó de imprimir */
    sem_t C; /* Mutex para evitar inanición del máster al acceder al estado */
    sem_t D; /* Mutex para el estado del juego */
    sem_t E; /* Mutex para la siguiente variable */
    unsigned int F; /* Cantidad de jugadores leyendo el estado */
    sem_t G[9]; /* Semáforos por jugador */
} game_sync_t;

int main(){

    int fd;
    game_sync_t* ptr = (game_sync_t*) shm_init("/game_sync", sizeof(game_sync_t), &fd, O_RDWR);

    sem_init(&ptr->A, 1, 0);
    sem_init(&ptr->B, 1, 0);

    ptr->F = 1;

    pid_t pid = fork();

    if (pid == 0){
        char* argv[] = {"vista_prueba", NULL};
        execv("./vista_prueba", argv);
    }else{
        // Proceso padre
        while (ptr->F) {
            // Simular cambios en el estado del juego
            printf("Estoy en el padre cambiando el estado.\n");
            sleep(3);
            printf("Necesito imprimir.\n");
            sem_post(&ptr->A); // Indica que hay cambios
            sem_wait(&ptr->B); // Espera a que el hijo termine de imprimir
        }
    }

    sem_destroy(&ptr->A);
    sem_destroy(&ptr->B);

    shm_close(ptr, sizeof(game_sync_t), fd);
    shm_destroy("/game_sync");

    return 0;
}

