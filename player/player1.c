// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com 
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
    // Greedy: mira las 8 casillas alrededor (incluye diagonales) y elige la de mayor valor.
    int width = state->width;
    int height = state->height;
    int x = player[me].x_coord;
    int y = player[me].y_coord;

    int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};

    // Mapear los indices anteriores a los códigos requeridos según el enunciado:
    // Coordenadas deseadas:
    // 7 0 1
    // 6   2
    // 5 4 3
    int code_map[] = {7, 0, 1, 6, 2, 5, 4, 3};

    int best_move = 0;
    int best_value = -2147483648; // valor muy pequeño

    for (int move = 0; move < 8; move++) {
        int nx = x + dx[move];
        int ny = y + dy[move];
        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
            int idx = ny * width + nx;
            int value = state->board_data[idx];
            if (value > best_value) {
                best_value = value;
                best_move = move;
            }
        }
    }

    return code_map[best_move];
}
