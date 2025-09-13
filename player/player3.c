// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <assert.h>

#include "../common/game_state.h"
#include "../common/game_sync.h"
#include "../shared_memory/shm.h"
#include "player_lib.h"

// Direcciones: 0=Norte, 1=Noreste, 2=Este, 3=Sureste, 4=Sur, 5=Suroeste, 6=Oeste, 7=Noroeste
static const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

#define MAX_SIZE 50
#define INF 9999
#define max(a,b) ((a) > (b) ? (a) : (b))

// Máquina de estados mejorada
typedef enum {
    FASE_GO_TO_BORDER,
    FASE_PERIMETRO,
    FASE_RELLENO
} estrategia_fase_t;

// Variables estáticas para mantener estado entre llamadas
static estrategia_fase_t fase_actual = FASE_GO_TO_BORDER;
static int tau = 2;  // Ventaja mínima requerida
static int D_self[MAX_SIZE][MAX_SIZE];
static int D_others[MAX_SIZE][MAX_SIZE];
static int D_objetivo[MAX_SIZE][MAX_SIZE];  // Distancias al objetivo
static int M[MAX_SIZE][MAX_SIZE];  // Matriz de ventaja
static int region_segura[MAX_SIZE][MAX_SIZE];  // 1 si está en región segura
static int contorno[MAX_SIZE][MAX_SIZE];  // 1 si es contorno
static int corredor[MAX_SIZE][MAX_SIZE];  // 1 si es corredor al borde
static int dir_actual = 2;  // Dirección actual para wall following
static int perimetro_cerrado = 0;
static int estoy_adentro = 0;  // Flag para saber si estoy dentro del perímetro
static int pos_inicio_x = -1, pos_inicio_y = -1;
static int objetivo_x = -1, objetivo_y = -1;  // Objetivo del borde
static int relleno_direccion = 2;
static int relleno_horizontal = 1;  // 1 = horizontal, 0 = vertical
static int relleno_ida = 1;

// Función auxiliar: verificar si un movimiento es válido
int esMovimientoValido(game_state_t* state, int x, int y, int direccion) {
    int nx = x + dx[direccion];
    int ny = y + dy[direccion];
    
    if (nx < 0 || nx >= state->width || ny < 0 || ny >= state->height) {
        return 0;
    }
    
    int idx = ny * state->width + nx;
    return state->board_data[idx] > 0;
}

// Función auxiliar: verificar si una celda es transitable (incluyendo mi posición)
int esTransitable(game_state_t* state, player_t* players, int me, int x, int y) {
    if (x < 0 || x >= state->width || y < 0 || y >= state->height) {
        return 0;
    }
    
    // Mi posición actual es transitable
    if (x == players[me].x_coord && y == players[me].y_coord) {
        return 1;
    }
    
    int idx = y * state->width + x;
    return state->board_data[idx] > 0;
}

// Función auxiliar: BFS para calcular distancias (versión mejorada)
void calcularDistanciasBFS(game_state_t* state, player_t* players, int me, 
                          int start_x, int start_y, int dist[MAX_SIZE][MAX_SIZE], int usar_mi_pos) {
    assert(state->width <= MAX_SIZE && state->height <= MAX_SIZE);
    
    // Inicializar todas las distancias a INF
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            dist[x][y] = INF;
        }
    }
    
    // Cola simple para BFS
    int queue_x[MAX_SIZE * MAX_SIZE];
    int queue_y[MAX_SIZE * MAX_SIZE];
    int front = 0, rear = 0;
    
    // Inicializar con posición de inicio
    dist[start_x][start_y] = 0;
    queue_x[rear] = start_x;
    queue_y[rear] = start_y;
    rear++;
    
    while (front < rear) {
        int cx = queue_x[front];
        int cy = queue_y[front];
        front++;
        
        // Explorar 8 direcciones
        for (int dir = 0; dir < 8; dir++) {
            int nx = cx + dx[dir];
            int ny = cy + dy[dir];
            
            if (nx >= 0 && nx < state->width && ny >= 0 && ny < state->height && dist[nx][ny] == INF) {
                int transitable = usar_mi_pos ? 
                    esTransitable(state, players, me, nx, ny) : 
                    (state->board_data[ny * state->width + nx] > 0);
                
                if (transitable) {
                    dist[nx][ny] = dist[cx][cy] + 1;
                    queue_x[rear] = nx;
                    queue_y[rear] = ny;
                    rear++;
                }
            }
        }
    }
}

// Función auxiliar: calcular región segura R(τ) mejorada
void calcularRegionSegura(game_state_t* state, player_t* players, int me) {
    int mi_x = players[me].x_coord;
    int mi_y = players[me].y_coord;
    
    // Calcular distancias desde mi posición (incluyendo mi celda actual)
    calcularDistanciasBFS(state, players, me, mi_x, mi_y, D_self, 1);
    
    // Inicializar D_others con INF
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            D_others[x][y] = INF;
        }
    }
    
    // Calcular distancias mínimas desde todos los otros jugadores activos
    for (int i = 0; i < state->player_count; i++) {
        if (i != me && !players[i].is_blocked) {
            int temp_dist[MAX_SIZE][MAX_SIZE];
            calcularDistanciasBFS(state, players, me, players[i].x_coord, players[i].y_coord, temp_dist, 0);
            
            // Tomar el mínimo entre D_others y temp_dist
            for (int y = 0; y < state->height; y++) {
                for (int x = 0; x < state->width; x++) {
                    if (temp_dist[x][y] < D_others[x][y]) {
                        D_others[x][y] = temp_dist[x][y];
                    }
                }
            }
        }
    }
    
    // Calcular matriz de ventaja M y región segura
    int celdas_en_region = 0;
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            if (esTransitable(state, players, me, x, y)) {
                M[x][y] = D_others[x][y] - D_self[x][y];
                if (M[x][y] >= tau) {
                    region_segura[x][y] = 1;
                    celdas_en_region++;
                } else {
                    region_segura[x][y] = 0;
                }
            } else {
                region_segura[x][y] = 0;
                M[x][y] = -INF;
            }
        }
    }
    
    // Si la región es muy pequeña, reducir tau
    if (celdas_en_region < 10 && tau > 0) {
        tau--;
        calcularRegionSegura(state, players, me);
        return;
    }
}

// Función auxiliar: extraer contorno y buscar ancla al borde
void calcularContornoYAncla(game_state_t* state) {
    // Inicializar contorno
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            contorno[x][y] = 0;
            corredor[x][y] = 0;
        }
    }
    
    // Calcular contorno de la región segura
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            if (region_segura[x][y] == 1) {
                for (int dir = 0; dir < 8; dir++) {
                    int nx = x + dx[dir];
                    int ny = y + dy[dir];
                    
                    if (nx < 0 || nx >= state->width || ny < 0 || ny >= state->height || 
                        region_segura[nx][ny] == 0) {
                        contorno[x][y] = 1;
                        break;
                    }
                }
            }
        }
    }
    
    // Buscar ancla al borde desde el contorno
    int mejor_ancla_x = -1, mejor_ancla_y = -1;
    int mejor_area = 0;
    
    for (int y = 0; y < state->height; y++) {
        for (int x = 0; x < state->width; x++) {
            // Verificar si es borde del tablero y está en contorno o accesible
            if ((x == 0 || x == state->width - 1 || y == 0 || y == state->height - 1)) {
                if (contorno[x][y] == 1 || (M[x][y] >= tau - 1 && esMovimientoValido(state, x, y, 0))) {
                    // Estimar área potencial desde este punto
                    int area_estimada = 0;
                    for (int dy = -5; dy <= 5; dy++) {
                        for (int dx = -5; dx <= 5; dx++) {
                            int nx = x + dx;
                            int ny = y + dy;
                            if (nx >= 0 && nx < state->width && ny >= 0 && ny < state->height && 
                                region_segura[nx][ny] == 1) {
                                area_estimada++;
                            }
                        }
                    }
                    
                    if (area_estimada > mejor_area) {
                        mejor_area = area_estimada;
                        mejor_ancla_x = x;
                        mejor_ancla_y = y;
                    }
                }
            }
        }
    }
    
    // Si encontramos ancla, marcar corredor
    if (mejor_ancla_x != -1) {
        objetivo_x = mejor_ancla_x;
        objetivo_y = mejor_ancla_y;
        
        // Marcar corredor con τ-1 si es necesario
        for (int y = 0; y < state->height; y++) {
            for (int x = 0; x < state->width; x++) {
                if (M[x][y] >= tau - 1) {
                    corredor[x][y] = 1;
                }
            }
        }
        
        // Calcular distancias al objetivo
        calcularDistanciasBFS(state, NULL, -1, objetivo_x, objetivo_y, D_objetivo, 0);
    }
}

// Función auxiliar: wall following con mano izquierda
int wallFollowing(game_state_t* state, int x, int y) {
    // Orden de prueba para mano izquierda: girar 45° izq, recto, girar 45° der, etc.
    int orden_prueba[8];
    for (int i = 0; i < 8; i++) {
        orden_prueba[i] = (dir_actual + 7 + i) % 8;  // Empezar por la izquierda
    }
    
    for (int i = 0; i < 8; i++) {
        int dir = orden_prueba[i];
        int nx = x + dx[dir];
        int ny = y + dy[dir];
        
        // Verificar si es movimiento válido y dentro de región/corredor
        if (esMovimientoValido(state, x, y, dir)) {
            if (region_segura[nx][ny] == 1 || corredor[nx][ny] == 1) {
                dir_actual = dir;
                return dir;
            }
        }
    }
    
    // Fallback: cualquier movimiento válido
    for (int dir = 0; dir < 8; dir++) {
        if (esMovimientoValido(state, x, y, dir)) {
            dir_actual = dir;
            return dir;
        }
    }
    
    return 0;
}

// Función auxiliar: verificar si puedo cerrar el perímetro con criterio seguro
int puedoCerrarPerimetroSeguro(game_state_t* state, int x, int y) {
    if (!estoy_adentro || pos_inicio_x == -1 || pos_inicio_y == -1) {
        return 0;
    }
    
    // Usar distancia Chebyshev
    int dist_chebyshev = max(abs(x - pos_inicio_x), abs(y - pos_inicio_y));
    if (dist_chebyshev > 1) {
        return 0;
    }
    
    // Verificar que el cierre no corta el interior
    for (int dir = 0; dir < 8; dir++) {
        if (esMovimientoValido(state, x, y, dir)) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            
            // Verificar si es adyacente al inicio
            int dist_inicio = max(abs(nx - pos_inicio_x), abs(ny - pos_inicio_y));
            if (dist_inicio <= 1) {
                // Simular el movimiento y verificar libertades internas
                int libertades_internas = 0;
                for (int check_dir = 0; check_dir < 8; check_dir++) {
                    int check_x = nx + dx[check_dir];
                    int check_y = ny + dy[check_dir];
                    if (esMovimientoValido(state, nx, ny, check_dir) && 
                        region_segura[check_x][check_y] == 1) {
                        libertades_internas++;
                    }
                }
                
                if (libertades_internas >= 1) {
                    return dir;
                }
            }
        }
    }
    
    return -1;  // No se puede cerrar de forma segura
}

// Función para FASE_GO_TO_BORDER
int faseGoToBorder(game_state_t* state, player_t* players, int me) {
    int mi_x = players[me].x_coord;
    int mi_y = players[me].y_coord;
    
    // Calcular región segura y ancla
    calcularRegionSegura(state, players, me);
    calcularContornoYAncla(state);
    
    // Si no hay objetivo o ya llegamos, cambiar a FASE_PERIMETRO
    if (objetivo_x == -1 || objetivo_y == -1 || 
        (mi_x == objetivo_x && mi_y == objetivo_y)) {
        fase_actual = FASE_PERIMETRO;
        pos_inicio_x = mi_x;
        pos_inicio_y = mi_y;
        return fasePerimetro(state, players, me);
    }
    
    // Moverse hacia el objetivo usando distancia 8-vecinos
    int mejor_direccion = 0;
    int menor_distancia = INF;
    
    for (int dir = 0; dir < 8; dir++) {
        if (esMovimientoValido(state, mi_x, mi_y, dir)) {
            int nx = mi_x + dx[dir];
            int ny = mi_y + dy[dir];
            
            // Preferir movimientos dentro del corredor/región
            if ((corredor[nx][ny] == 1 || region_segura[nx][ny] == 1) && 
                D_objetivo[nx][ny] < menor_distancia) {
                menor_distancia = D_objetivo[nx][ny];
                mejor_direccion = dir;
            }
        }
    }
    
    return mejor_direccion;
}

// Función para FASE_PERIMETRO
int fasePerimetro(game_state_t* state, player_t* players, int me) {
    int mi_x = players[me].x_coord;
    int mi_y = players[me].y_coord;
    
    // Recalcular contorno cada turno
    calcularRegionSegura(state, players, me);
    calcularContornoYAncla(state);
    
    // Verificar si estoy adentro (una celda hacia el interior desde pared exterior)
    if (!estoy_adentro) {
        int celdas_ocupadas_alrededor = 0;
        int celdas_totales = 0;
        for (int dir = 0; dir < 8; dir++) {
            int nx = mi_x + dx[dir];
            int ny = mi_y + dy[dir];
            if (nx >= 0 && nx < state->width && ny >= 0 && ny < state->height) {
                celdas_totales++;
                if (!esMovimientoValido(state, mi_x, mi_y, dir)) {
                    celdas_ocupadas_alrededor++;
                }
            }
        }
        if (celdas_ocupadas_alrededor * 2 > celdas_totales) {
            estoy_adentro = 1;
        }
    }
    
    // Verificar cierre seguro
    int dir_cierre = puedoCerrarPerimetroSeguro(state, mi_x, mi_y);
    if (dir_cierre >= 0) {
        perimetro_cerrado = 1;
        fase_actual = FASE_RELLENO;
        
        // Determinar orientación del relleno según área
        int ancho_util = 0, alto_util = 0;
        for (int y = 0; y < state->height; y++) {
            for (int x = 0; x < state->width; x++) {
                if (region_segura[x][y] == 1) {
                    if (x > ancho_util) ancho_util = x;
                    if (y > alto_util) alto_util = y;
                }
            }
        }
        relleno_horizontal = (ancho_util > alto_util) ? 1 : 0;
        relleno_direccion = relleno_horizontal ? 2 : 4;  // Este o Sur
        
        return dir_cierre;
    }
    
    // Seguir construyendo perímetro con wall following
    return wallFollowing(state, mi_x, mi_y);
}

// Función para FASE_RELLENO con regla ≥2 salidas
int faseRelleno(game_state_t* state, int x, int y) {
    // Contar salidas actuales
    int salidas = 0;
    for (int dir = 0; dir < 8; dir++) {
        if (esMovimientoValido(state, x, y, dir)) {
            salidas++;
        }
    }
    
    // Si tengo solo 1 salida y queda área por cubrir, evitar el bolsillo
    if (salidas <= 1) {
        // Buscar movimiento que mantenga más salidas
        int mejor_dir = 0;
        int mejores_salidas = 0;
        
        for (int dir = 0; dir < 8; dir++) {
            if (esMovimientoValido(state, x, y, dir)) {
                int nx = x + dx[dir];
                int ny = y + dy[dir];
                int nuevas_salidas = 0;
                
                for (int check_dir = 0; check_dir < 8; check_dir++) {
                    if (esMovimientoValido(state, nx, ny, check_dir)) {
                        nuevas_salidas++;
                    }
                }
                
                if (nuevas_salidas > mejores_salidas) {
                    mejores_salidas = nuevas_salidas;
                    mejor_dir = dir;
                }
            }
        }
        
        if (mejores_salidas >= 2) {
            relleno_direccion = mejor_dir;
            return mejor_dir;
        }
    }
    
    // Patrón boustrophedon normal
    if (esMovimientoValido(state, x, y, relleno_direccion)) {
        return relleno_direccion;
    }
    
    // Cambiar de fila/columna
    int direccion_perpendicular;
    if (relleno_horizontal) {
        direccion_perpendicular = relleno_ida ? 4 : 0;  // Sur o Norte
    } else {
        direccion_perpendicular = relleno_ida ? 2 : 6;  // Este u Oeste
    }
    
    if (esMovimientoValido(state, x, y, direccion_perpendicular)) {
        relleno_ida = !relleno_ida;
        relleno_direccion = relleno_horizontal ? (relleno_ida ? 6 : 2) : (relleno_ida ? 0 : 4);
        return direccion_perpendicular;
    }
    
    // Fallback
    for (int dir = 0; dir < 8; dir++) {
        if (esMovimientoValido(state, x, y, dir)) {
            return dir;
        }
    }
    
    return 0;
}

// Función greedy segura como fallback
int greedy_seguro(game_state_t* state, int x, int y) {
    int mejor_dir = 0;
    int mejor_score = -1;
    
    for (int dir = 0; dir < 8; dir++) {
        if (esMovimientoValido(state, x, y, dir)) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            int salidas = 0;
            
            // Contar salidas desde la nueva posición
            for (int check_dir = 0; check_dir < 8; check_dir++) {
                if (esMovimientoValido(state, nx, ny, check_dir)) {
                    salidas++;
                }
            }
            
            if (salidas > mejor_score) {
                mejor_score = salidas;
                mejor_dir = dir;
            }
        }
    }
    
    return mejor_dir;
}

// Declaraciones forward para evitar errores de compilación
int fasePerimetro(game_state_t* state, player_t* players, int me);

// getMove debe ser implementada por cada jugador. Se mantiene aquí la lógica
// específica del jugador; la función main común ahora está en player_lib.c.
int getMove(player_t* player, game_state_t* state, int me) {
    int mi_x = player[me].x_coord;
    int mi_y = player[me].y_coord;
    
    // Verificar dimensiones de seguridad
    assert(state->width <= MAX_SIZE && state->height <= MAX_SIZE);
    
    // Si la región segura está vacía, usar greedy seguro
    if (tau <= 0) {
        return greedy_seguro(state, mi_x, mi_y);
    }
    
    switch (fase_actual) {
        case FASE_GO_TO_BORDER:
            return faseGoToBorder(state, player, me);
            
        case FASE_PERIMETRO:
            return fasePerimetro(state, player, me);
            
        case FASE_RELLENO:
            return faseRelleno(state, mi_x, mi_y);
            
        default:
            return greedy_seguro(state, mi_x, mi_y);
    }
}