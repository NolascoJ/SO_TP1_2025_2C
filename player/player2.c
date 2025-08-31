#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#include "../utils/game_state.h"
#include "../utils/game_sync.h"
#include "../shared_memory/shm.h"
#include "player_lib.h"

// getMove debe ser implementada por cada jugador. Se mantiene aquí la lógica
// específica del jugador; la función main común ahora está en player_lib.c.
int getMove(player_t* player, game_state_t* state, int me) {
   return 6; 
}
