#ifndef VIEW_H
#define VIEW_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <ncurses.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include "../common/game_state.h"
#include "../common/game_sync.h"
#include "../shared_memory/shm.h"

// UI Layout Constants
#define DEFAULT_LEADERBOARD_WIDTH 44
#define DEFAULT_MATRIX_CELL_WIDTH 4
#define LEADERBOARD_START_Y 1
#define LEADERBOARD_MATRIX_SPACING 1
#define WINDOW_BORDER_SIZE 2

// Leaderboard Column Positions
#define NAME_COLUMN_POS 2
#define SCORE_COLUMN_POS 18
#define VALID_COLUMN_POS 26
#define INVALID_COLUMN_POS 34
#define NAME_FIELD_WIDTH 14

// Color Pair Definitions
#define MAX_PLAYERS 9
#define PLAYER_COLOR_RED 1
#define PLAYER_COLOR_GREEN 2
#define PLAYER_COLOR_YELLOW 3
#define PLAYER_COLOR_BLUE 4
#define PLAYER_COLOR_MAGENTA 5
#define PLAYER_COLOR_CYAN 6
#define PLAYER_COLOR_PURPLE 7
#define PLAYER_COLOR_DARK_GREEN 8
#define PLAYER_COLOR_ORANGE 9

// Final Scoreboard Layout Constants
#define FINAL_SCOREBOARD_WIDTH 60
#define FINAL_SCOREBOARD_EXTRA_HEIGHT 8
#define FINAL_TITLE_ROW 1
#define FINAL_HEADER_ROW 3
#define FINAL_SEPARATOR_ROW 4
#define FINAL_PLAYERS_START_ROW 5
#define FINAL_RANK_COLUMN 3
#define FINAL_PLAYER_COLUMN 10
#define FINAL_SCORE_COLUMN 35
#define FINAL_PLAYER_NAME_WIDTH 20
#define GAME_OVER_TEXT_LENGTH 11
#define PRESS_KEY_TEXT_LENGTH 25

// General Layout Constants  
#define EXPECTED_ARGC 3
#define REQUIRED_ARGS 2
#define LEADERBOARD_HEADER_ROWS 3
#define CURSOR_HIDDEN 0
#define TERM_256_COLOR 1
#define EXIT_SUCCESS_CODE 0
#define EXIT_ERROR_CODE 1
#define SEM_WAIT_SUCCESS 0

// Player Position Display
#define PLAYER_MARKER_CHAR "  P"
#define NUMBER_FORMAT "%3d"
#define SCORE_FORMAT "%6u"
#define MOVES_FORMAT "%4u"

void draw_leaderboard(WINDOW* win, const game_state_t* game_state_ptr);

void draw_matrix(WINDOW* win, const game_state_t* game_state_ptr);

void handle_adaptive_resize(int *scr_h, int *scr_w, int game_width, int game_height, 
                           const game_state_t* gs_ptr, WINDOW **leaderboard_win, WINDOW **matrix_win);
void init_player_colors(void);

void draw_final_scoreboard(const game_state_t* game_state_ptr);

typedef struct {
    unsigned int idx;
    unsigned int score;
    unsigned int valid_moves;
    unsigned int invalid_moves;
} player_idx_t;
int compare_player_idx_desc(const void* a, const void* b);

#endif
