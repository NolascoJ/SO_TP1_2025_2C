// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com 

/*
 * Player 2 Strategy: Advanced Cluster-Based Heuristic with Isolation Penalty
 * 
 * This player implements a sophisticated strategy that balances:
 * 1. Immediate cell value collection
 * 2. Cluster proximity evaluation (nearby high-value cells)
 * 3. Isolation penalty (avoiding positions with few escape routes)
 * 4. Solo mode optimization (when no other players are active)
 * 
 * Heuristic Parameters:
 * - ALPHA: Weight for cluster proximity value
 * - GAMMA: Penalty coefficient for isolation
 * - CLUSTER_RADIUS: Range for cluster evaluation
 * - MIN_FREE_CELLS: Threshold for isolation detection
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#include "../common/game_state.h"
#include "../common/game_sync.h"
#include "../shared_memory/shm.h"
#include "player_lib.h"

#define ALPHA 0.8f      
#define GAMMA 1.0f      
#define CLUSTER_RADIUS 3
#define MIN_FREE_CELLS 2

// Directions: 0=North, 1=Northeast, 2=East, 3=Southeast, 4=South, 5=Southwest, 6=West, 7=Northwest
static const int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
static const int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

int isValidMove(game_state_t* state, int x, int y, int direction) {
    int nx = x + dx[direction];
    int ny = y + dy[direction];
    
    if (nx < 0 || nx >= state->width || ny < 0 || ny >= state->height) {
        return 0;
    }
    
    int idx = ny * state->width + nx;
    return state->board_data[idx] > 0;
}

int getCellValue(game_state_t* state, int x, int y) {
    if (x < 0 || x >= state->width || y < 0 || y >= state->height) {
        return 0;
    }
    int idx = y * state->width + x;
    return state->board_data[idx] > 0 ? state->board_data[idx] : 0;
}

int countFreeCells(game_state_t* state, int x, int y) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < state->width && ny >= 0 && ny < state->height) {
            int idx = ny * state->width + nx;
            if (state->board_data[idx] > 0) {
                count++;
            }
        }
    }
    return count;
}

float calculateNearbyCluster(game_state_t* state, int x, int y) {
    float cluster_value = 0.0f;
    
    for (int r = 1; r <= CLUSTER_RADIUS; r++) {
        for (int dx_r = -r; dx_r <= r; dx_r++) {
            for (int dy_r = -r; dy_r <= r; dy_r++) {
                if (abs(dx_r) == r || abs(dy_r) == r) { // Only the radius border
                    int nx = x + dx_r;
                    int ny = y + dy_r;
                    int value = getCellValue(state, nx, ny);
                    if (value > 0) {
                        float distance = (float)r;
                        cluster_value += value / distance;
                    }
                }
            }
        }
    }
    
    return cluster_value;
}

float calculateScore(game_state_t* state, int x, int y, int direction) {
    if (!isValidMove(state, x, y, direction)) {
        return -1000000.0f; // Invalid move
    }
    
    int nx = x + dx[direction];
    int ny = y + dy[direction];
    
    float immediate_value = (float)getCellValue(state, nx, ny);
    
    int free_cells_after = countFreeCells(state, nx, ny) - 1; // -1 because we'll occupy current cell
    float isolation_penalty = 0.0f;
    if (free_cells_after <= MIN_FREE_CELLS) {
        isolation_penalty = GAMMA * (MIN_FREE_CELLS - free_cells_after + 1) * 10.0f;
    }
    
    float nearby_cluster = calculateNearbyCluster(state, nx, ny);
    
    return immediate_value + ALPHA * nearby_cluster - isolation_penalty;
}

int isInSoloMode(player_t* players, int player_count, int me) {
    for (int i = 0; i < player_count; i++) {
        if (i != me && !players[i].is_blocked) {
            return 0; 
        }
    }
    return 1; 
}

int findBestHighValueCell(game_state_t* state, int x, int y, int* best_x, int* best_y) {
    int best_value = 0;
    *best_x = x;
    *best_y = y;
    
    for (int radius = 1; radius <= 5; radius++) {
        for (int dx_r = -radius; dx_r <= radius; dx_r++) {
            for (int dy_r = -radius; dy_r <= radius; dy_r++) {
                int nx = x + dx_r;
                int ny = y + dy_r;
                int value = getCellValue(state, nx, ny);
                if (value >= 7 && value > best_value) { // High-value cells
                    best_value = value;
                    *best_x = nx;
                    *best_y = ny;
                }
            }
        }
        if (best_value > 0) break;
    }
    
    return best_value;
}

int soloMode(game_state_t* state, int x, int y) {
    int best_x, best_y;
    int best_value = findBestHighValueCell(state, x, y, &best_x, &best_y);
    
    if (best_value > 0) {
        int best_direction = 0;
        float best_score = -1000000.0f;
        
        for (int dir = 0; dir < 8; dir++) {
            if (isValidMove(state, x, y, dir)) {
                int nx = x + dx[dir];
                int ny = y + dy[dir];
                
                int current_dist = abs(x - best_x) + abs(y - best_y);
                int new_dist = abs(nx - best_x) + abs(ny - best_y);
                
                float score = calculateScore(state, x, y, dir);
                
                if (new_dist < current_dist) {
                    score += 5.0f;
                }
                
                if (score > best_score) {
                    best_score = score;
                    best_direction = dir;
                }
            }
        }
        return best_direction;
    }
    
    int best_direction = 0;
    float best_score = -1000000.0f;
    
    for (int dir = 0; dir < 8; dir++) {
        float score = calculateScore(state, x, y, dir);
        if (score > best_score) {
            best_score = score;
            best_direction = dir;
        }
    }
    
    return best_direction;
}

int getMove(player_t* player, game_state_t* state, int me) {
    int x = player[me].x_coord;
    int y = player[me].y_coord;
    
    if (isInSoloMode(player, state->player_count, me)) {
        return soloMode(state, x, y);
    }
    
    int best_direction = 0;
    float best_score = -1000000.0f;
    
    for (int dir = 0; dir < 8; dir++) {
        float score = calculateScore(state, x, y, dir);
        if (score > best_score) {
            best_score = score;
            best_direction = dir;
        }
    }
    
    if (best_score <= -1000000.0f) {
        return 0;
    }
    
    return best_direction;
}
