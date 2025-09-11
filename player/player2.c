/* // This is a personal academic project. Dear PVS-Studio, please check it.
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
   return 6; 
} */
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

// // Parámetros de la heurística
// #define ALPHA 0.8f      // Peso del cluster cercano
// #define GAMMA 1.0f      // Penalización por encierro
// #define CLUSTER_RADIUS 3
// #define MIN_FREE_CELLS 2

// // Direcciones: 0=Norte, 1=Noreste, 2=Este, 3=Sureste, 4=Sur, 5=Suroeste, 6=Oeste, 7=Noroeste
// static const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
// static const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

// // Función auxiliar: verificar si un movimiento es válido
// int esMovimientoValido(game_state_t* state, int x, int y, int direccion) {
//     int nx = x + dx[direccion];
//     int ny = y + dy[direccion];
    
//     if (nx < 0 || nx >= state->width || ny < 0 || ny >= state->height) {
//         return 0;
//     }
    
//     int idx = ny * state->width + nx;
//     return state->board_data[idx] > 0;
// }

// // Función auxiliar: obtener valor de una celda
// int obtenerValorCelda(game_state_t* state, int x, int y) {
//     if (x < 0 || x >= state->width || y < 0 || y >= state->height) {
//         return 0;
//     }
//     int idx = y * state->width + x;
//     return state->board_data[idx] > 0 ? state->board_data[idx] : 0;
// }

// // Función auxiliar: contar celdas libres alrededor de una posición
// int contarCeldasLibres(game_state_t* state, int x, int y) {
//     int count = 0;
//     for (int i = 0; i < 8; i++) {
//         int nx = x + dx[i];
//         int ny = y + dy[i];
//         if (nx >= 0 && nx < state->width && ny >= 0 && ny < state->height) {
//             int idx = ny * state->width + nx;
//             if (state->board_data[idx] > 0) {
//                 count++;
//             }
//         }
//     }
//     return count;
// }

// // Función auxiliar: calcular valor del cluster cercano
// float calcularClusterCercano(game_state_t* state, int x, int y) {
//     float cluster_value = 0.0f;
    
//     for (int r = 1; r <= CLUSTER_RADIUS; r++) {
//         for (int dx_r = -r; dx_r <= r; dx_r++) {
//             for (int dy_r = -r; dy_r <= r; dy_r++) {
//                 if (abs(dx_r) == r || abs(dy_r) == r) { // Solo el borde del radio
//                     int nx = x + dx_r;
//                     int ny = y + dy_r;
//                     int valor = obtenerValorCelda(state, nx, ny);
//                     if (valor > 0) {
//                         float distancia = (float)r;
//                         cluster_value += valor / distancia;
//                     }
//                 }
//             }
//         }
//     }
    
//     return cluster_value;
// }

// // Función auxiliar: calcular score de un movimiento
// float calcularScore(game_state_t* state, int x, int y, int direccion) {
//     if (!esMovimientoValido(state, x, y, direccion)) {
//         return -1000000.0f; // Movimiento inválido
//     }
    
//     int nx = x + dx[direccion];
//     int ny = y + dy[direccion];
    
//     // Valor inmediato de la celda destino
//     float valor_inmediato = (float)obtenerValorCelda(state, nx, ny);
    
//     // Penalización por encierro
//     int celdas_libres_despues = contarCeldasLibres(state, nx, ny) - 1; // -1 porque ocuparemos la celda actual
//     float penalizacion_encierro = 0.0f;
//     if (celdas_libres_despues <= MIN_FREE_CELLS) {
//         penalizacion_encierro = GAMMA * (MIN_FREE_CELLS - celdas_libres_despues + 1) * 10.0f;
//     }
    
//     // Valor del cluster cercano
//     float cluster_cercano = calcularClusterCercano(state, nx, ny);
    
//     return valor_inmediato + ALPHA * cluster_cercano - penalizacion_encierro;
// }

// // Función auxiliar: detectar si estoy en modo solo
// int estoyEnModoSolo(player_t* players, int player_count, int me) {
//     for (int i = 0; i < player_count; i++) {
//         if (i != me && !players[i].is_blocked) {
//             return 0; // Hay otros jugadores activos
//         }
//     }
//     return 1; // Solo yo estoy activo
// }

// // Función auxiliar: encontrar la mejor celda de alto valor cercana (modo solo)
// int encontrarMejorCeldaAlta(game_state_t* state, int x, int y, int* mejor_x, int* mejor_y) {
//     int mejor_valor = 0;
//     *mejor_x = x;
//     *mejor_y = y;
    
//     // Buscar en un radio amplio
//     for (int radio = 1; radio <= 5; radio++) {
//         for (int dx_r = -radio; dx_r <= radio; dx_r++) {
//             for (int dy_r = -radio; dy_r <= radio; dy_r++) {
//                 int nx = x + dx_r;
//                 int ny = y + dy_r;
//                 int valor = obtenerValorCelda(state, nx, ny);
//                 if (valor >= 7 && valor > mejor_valor) { // Celdas de alto valor
//                     mejor_valor = valor;
//                     *mejor_x = nx;
//                     *mejor_y = ny;
//                 }
//             }
//         }
//         if (mejor_valor > 0) break; // Encontramos algo bueno
//     }
    
//     return mejor_valor;
// }

// // Función auxiliar: modo solo - estrategia avanzada
// int modoSolo(game_state_t* state, int x, int y) {
//     int mejor_x, mejor_y;
//     int mejor_valor = encontrarMejorCeldaAlta(state, x, y, &mejor_x, &mejor_y);
    
//     if (mejor_valor > 0) {
//         // Moverse hacia la celda de alto valor
//         int mejor_direccion = 0;
//         float mejor_score = -1000000.0f;
        
//         for (int dir = 0; dir < 8; dir++) {
//             if (esMovimientoValido(state, x, y, dir)) {
//                 int nx = x + dx[dir];
//                 int ny = y + dy[dir];
                
//                 // Calcular distancia Manhattan hacia el objetivo
//                 int dist_actual = abs(x - mejor_x) + abs(y - mejor_y);
//                 int dist_nueva = abs(nx - mejor_x) + abs(ny - mejor_y);
                
//                 float score = calcularScore(state, x, y, dir);
                
//                 // Bonificación por acercarse al objetivo
//                 if (dist_nueva < dist_actual) {
//                     score += 5.0f;
//                 }
                
//                 if (score > mejor_score) {
//                     mejor_score = score;
//                     mejor_direccion = dir;
//                 }
//             }
//         }
//         return mejor_direccion;
//     }
    
//     // Si no hay celdas de alto valor, usar estrategia normal
//     int mejor_direccion = 0;
//     float mejor_score = -1000000.0f;
    
//     for (int dir = 0; dir < 8; dir++) {
//         float score = calcularScore(state, x, y, dir);
//         if (score > mejor_score) {
//             mejor_score = score;
//             mejor_direccion = dir;
//         }
//     }
    
//     return mejor_direccion;
// }

// // getMove debe ser implementada por cada jugador. Se mantiene aquí la lógica
// // específica del jugador; la función main común ahora está en player_lib.c.
// int getMove(player_t* player, game_state_t* state, int me) {
//     int x = player[me].x_coord;
//     int y = player[me].y_coord;
    
//     // Detectar si estoy en modo solo
//     if (estoyEnModoSolo(player, state->player_count, me)) {
//         return modoSolo(state, x, y);
//     }
    
//     // Modo normal: usar heurística rápida
//     int mejor_direccion = 0;
//     float mejor_score = -1000000.0f;
    
//     for (int dir = 0; dir < 8; dir++) {
//         float score = calcularScore(state, x, y, dir);
//         if (score > mejor_score) {
//             mejor_score = score;
//             mejor_direccion = dir;
//         }
//     }
    
//     // Si no hay movimientos válidos, retornar 0 (movimiento inválido controlado)
//     if (mejor_score <= -1000000.0f) {
//         return 0;
//     }
    
//     return mejor_direccion;
// }

int getMove(player_t* player, game_state_t* state, int me) {
    return 6;
}