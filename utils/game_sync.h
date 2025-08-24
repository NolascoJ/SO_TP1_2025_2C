#ifndef GAME_SYNC_H
#define GAME_SYNC_H

#include <semaphore.h>
#include <stdint.h>

#define GAME_SYNC_MAX_PLAYERS 9

typedef struct {
    sem_t A; /* El máster le indica a la vista que hay cambios por imprimir */
    sem_t B; /* La vista le indica al máster que terminó de imprimir */
    sem_t C; /* Mutex para evitar inanición del máster al acceder al estado */
    sem_t D; /* Mutex para el estado del juego */
    sem_t E; /* Mutex para la siguiente variable */
    unsigned int F; /* Cantidad de jugadores leyendo el estado */
    sem_t G[GAME_SYNC_MAX_PLAYERS]; /* Semáforos por jugador */
} game_sync_t;

#endif 