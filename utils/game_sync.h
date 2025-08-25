#ifndef GAME_SYNC_H
#define GAME_SYNC_H

#include <semaphore.h>
#include <stdint.h>

#define GAME_SYNC_MAX_PLAYERS 9

typedef struct {
    sem_t master_to_view; /* El máster le indica a la vista que hay cambios por imprimir */
    sem_t view_to_master; /* La vista le indica al máster que terminó de imprimir */
    sem_t master_mutex; /* Mutex para evitar inanición del máster al acceder al estado */
    sem_t game_state_mutex; /* Mutex para el estado del juego */
    sem_t readers_count_mutex; /* Mutex para la siguiente variable */
    unsigned int readers_count; /* Cantidad de jugadores leyendo el estado */
    sem_t player_semaphores[GAME_SYNC_MAX_PLAYERS]; /* Semáforos por jugador */
} game_sync_t;

#endif