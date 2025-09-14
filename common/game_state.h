#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

#define MAX_PLAYERS 9
#define MAX_NAME_LENGTH 16

typedef struct {
    char name[MAX_NAME_LENGTH];          
    unsigned int score;     
    unsigned int invalid_moves; 
    unsigned int valid_moves;   
    unsigned short x_coord;     
    unsigned short y_coord;     
    pid_t pid;              
    bool is_blocked;        
} player_t;

typedef struct {
    unsigned short width;   
    unsigned short height;  
    unsigned int player_count; 
    player_t players[MAX_PLAYERS];      
    bool game_over;         
    int board_data[];       
} game_state_t;

#endif