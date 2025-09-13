#ifndef VIEW_H
#define VIEW_H

#include <ncurses.h>
#include "../utils/game_state.h"





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

typedef struct { unsigned int idx; unsigned int score; } player_idx_t;
int compare_player_idx_desc(const void* a, const void* b);

#endif
