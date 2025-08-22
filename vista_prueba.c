#include "shared_memory/shm.h"
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

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
    game_sync_t* ptr = (game_sync_t*) shm_open_and_map("/game_sync", sizeof(game_sync_t), &fd, O_RDWR);

    for (int i = 0; i < 10; i++){
        sem_wait(&ptr->A); // espera a que el máster indique que hay cambios
        printf("Imprimiendo la pantallita\n");
        sleep(5);
        sem_post(&ptr->B); // Indica que terminó de imprimir
    }

    ptr->F = 0; 

    shm_close(ptr, sizeof(game_sync_t), fd);

    return 0;
}