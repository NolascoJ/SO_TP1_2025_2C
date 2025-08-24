#ifndef PLAYER_H
#define PLAYER_H

#include <stddef.h>
#include "../utils/game_state.h"
#include "../utils/game_sync.h"

// Limpia recursos de memoria compartida
void cleanup_resources(game_state_t* game_state_ptr, game_sync_t* game_sync_ptr, 
                      size_t state_size, size_t sync_size, int state_fd, int sync_fd);

// Obtiene el índice del jugador actual en el arreglo de jugadores
unsigned int getMe(game_state_t* game_state_ptr);

// Adquiere el lock de lectura usando el protocolo de sincronización
void acquire_read_lock(game_sync_t* game_sync_ptr, int me);

// Libera el lock de lectura
void release_read_lock(game_sync_t* game_sync_ptr);

// Toma un snapshot del estado del juego
void take_snapshot(game_state_t* game_state_ptr, player_t* playerList, game_state_t* state, int gameWidth, int gameHeight);

// Calcula el próximo movimiento basado en el estado del juego
int getMove(player_t* playerList, game_state_t* state, int me);

#endif
