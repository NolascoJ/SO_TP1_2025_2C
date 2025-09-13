#ifndef GAME_SYNC_H
#define GAME_SYNC_H

#include <semaphore.h>
#include <stdint.h>

#define GAME_SYNC_MAX_PLAYERS 9

typedef struct {
    sem_t master_to_view;
    sem_t view_to_master;
    sem_t master_mutex; // Prevents master starvation
    sem_t game_state_mutex;
    sem_t readers_count_mutex;
    unsigned int readers_count; 
    sem_t player_semaphores[GAME_SYNC_MAX_PLAYERS];
} game_sync_t;

#endif