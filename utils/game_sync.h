#ifndef GAME_SYNC_H
#define GAME_SYNC_H

#include <semaphore.h>
#include <stdint.h>

#define GAME_SYNC_MAX_PLAYERS 9

typedef struct {
    sem_t master_to_view; /* Master signals view that there are changes to print */
    sem_t view_to_master; /* View signals master that it finished printing */
    sem_t master_mutex; /* Mutex to prevent master starvation when accessing state */
    sem_t game_state_mutex; /* Mutex for game state */
    sem_t readers_count_mutex; /* Mutex for the following variable */
    unsigned int readers_count; /* Number of players reading the state */
    sem_t player_semaphores[GAME_SYNC_MAX_PLAYERS]; /* Semaphores per player */
} game_sync_t;

#endif