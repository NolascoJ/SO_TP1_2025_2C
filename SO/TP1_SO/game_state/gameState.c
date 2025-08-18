#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <stdbool.h>
#include <unistd.h> // Para pid_t

typedef struct {
    char name[16];          
    unsigned int score;     
    unsigned int invalid_moves; 
    unsigned int valid_moves;   
    unsigned short x_coord;     
    unsigned short y_coord;     
    pid_t pid;              
    bool is_blocked;        
} Player;

typedef struct {
    unsigned short width;   
    unsigned short height;  
    unsigned int player_count; 
    Player players[9];      
    bool game_over;         
    int board_data[];       
} GameState;

#endif