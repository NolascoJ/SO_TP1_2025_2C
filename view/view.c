
// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com 
#define _GNU_SOURCE
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
#include "../utils/game_state.h"
#include "../utils/game_sync.h"
#include "../shared_memory/shm.h"
#include "view.h"

// Global flag for window resize
static volatile sig_atomic_t resize_needed = 0;

// Signal handler for window resize
static void handle_resize(int sig) {
    (void)sig; // Unused parameter
    resize_needed = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "view2: expected 2 args: <width> <height>\n");
        return 1;
    }
    
    int game_width = atoi(argv[1]);
    int game_height = atoi(argv[2]);
    
    // Map game state
    size_t gs_size = sizeof(game_state_t) + (size_t)game_width * (size_t)game_height * sizeof(int);
    int gs_fd = -1;
    void* gs_map = shm_open_and_map("/game_state", gs_size, &gs_fd, O_RDONLY);
    if (gs_map == NULL) {
        return 1;
    }
    const game_state_t* gs_ptr = (const game_state_t*)gs_map;
    
    // Map sync
    size_t sync_size = sizeof(game_sync_t);
    int sync_fd = -1;
    void* sync_map = shm_open_and_map("/game_sync", sync_size, &sync_fd, O_RDWR);
    if (sync_map == NULL) {
        shm_close(gs_map, gs_size, gs_fd);
        return 1;
    }
    game_sync_t* sync_ptr = (game_sync_t*)sync_map;
    
    // Initialize ncurses
    if (getenv("TERM") == NULL) {
        setenv("TERM", "xterm-256color", 1);
    }
    
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    
    // Initialize colors
    init_player_colors();
    
    // Register signal handler for window resize
    signal(SIGWINCH, handle_resize);
    
    int scr_h, scr_w;
    getmaxyx(stdscr, scr_h, scr_w);
    
    // Layout: leaderboard at top, matrix below
    int leaderboard_h = (int)gs_ptr->player_count + 3;  // Header + actual players + borders
    int leaderboard_w = DEFAULT_LEADERBOARD_WIDTH;
    int leaderboard_y = LEADERBOARD_START_Y;
    int leaderboard_x = (scr_w - leaderboard_w) / 2;
    if (leaderboard_x < 0) leaderboard_x = 0;
    
    int matrix_h = game_height + WINDOW_BORDER_SIZE;  // Matrix + borders
    int matrix_w = game_width * DEFAULT_MATRIX_CELL_WIDTH + WINDOW_BORDER_SIZE;  // 4 chars per cell + borders
    int matrix_y = leaderboard_y + leaderboard_h + LEADERBOARD_MATRIX_SPACING;
    int matrix_x = (scr_w - matrix_w) / 2;
    if (matrix_x < 0) matrix_x = 0;
    
    WINDOW *leaderboard_win = newwin(leaderboard_h, leaderboard_w, leaderboard_y, leaderboard_x);
    WINDOW *matrix_win = newwin(matrix_h, matrix_w, matrix_y, matrix_x);

    bool is_over = false;
    
    // Main loop
    do{
        // Wait for master signal
        int sem_result = sem_wait(&sync_ptr->master_to_view);
        if (sem_result != 0) {
            // If interrupted by signal, continue the loop
            if (errno == EINTR) {
                continue;  // Go back to start of loop
            }
            break;  // Other errors, exit
        }
        
        // Handle window resize if needed
        if (resize_needed) {
            resize_needed = 0;
            handle_adaptive_resize(&scr_h, &scr_w, game_width, game_height, gs_ptr, &leaderboard_win, &matrix_win);
        }
        
        // Draw components
        draw_leaderboard(leaderboard_win, gs_ptr);
        draw_matrix(matrix_win, gs_ptr);
        
        // Refresh screens
        refresh();
        wrefresh(leaderboard_win);
        wrefresh(matrix_win);

        if ( gs_ptr->game_over) {
            is_over = true;
        }
        
        // Signal master that view is done
        sem_post(&sync_ptr->view_to_master);
        
        // Exit if game is over
  
    }while(!is_over);
    
    // Cleanup
    delwin(leaderboard_win);
    delwin(matrix_win);
    endwin();
    
    shm_close(sync_map, sync_size, sync_fd);
    shm_close(gs_map, gs_size, gs_fd);
    
    return 0;
}

void init_player_colors(void) {
    if (has_colors()) {
        start_color();
        init_pair(PLAYER_COLOR_RED,        COLOR_RED,     COLOR_BLACK);
        init_pair(PLAYER_COLOR_GREEN,      COLOR_GREEN,   COLOR_BLACK);
        init_pair(PLAYER_COLOR_YELLOW,     COLOR_YELLOW,  COLOR_BLACK);
        init_pair(PLAYER_COLOR_BLUE,       COLOR_BLUE,    COLOR_BLACK);
        init_pair(PLAYER_COLOR_MAGENTA,    COLOR_MAGENTA, COLOR_BLACK);
        init_pair(PLAYER_COLOR_CYAN,       COLOR_CYAN,    COLOR_BLACK);
        init_pair(PLAYER_COLOR_WHITE,      COLOR_BLUE,   COLOR_RED);
        init_pair(PLAYER_COLOR_RED_BLUE,   COLOR_RED,     COLOR_BLUE);
        init_pair(PLAYER_COLOR_GREEN_BLUE, COLOR_GREEN,   COLOR_BLUE);
    }
}

void draw_leaderboard(WINDOW* win, const game_state_t* game_state_ptr) {
    werase(win);
    box(win, 0, 0);
    
    // Header
    mvwprintw(win, 1, NAME_COLUMN_POS, "Name");
    mvwprintw(win, 1, SCORE_COLUMN_POS, "Score");
    mvwprintw(win, 1, VALID_COLUMN_POS, "Valid");
    mvwprintw(win, 1, INVALID_COLUMN_POS, "Invalid");
    
    // Player rows
    for (unsigned int i = 0; i < game_state_ptr->player_count && i < MAX_PLAYERS; i++) {
        const player_t* p = &game_state_ptr->players[i];
        int row = (int)i + 2;
        
        // Display player name with color
        wattron(win, COLOR_PAIR(i + 1) | A_BOLD);
        mvwprintw(win, row, NAME_COLUMN_POS, "%-*s", NAME_FIELD_WIDTH, p->name);
        wattroff(win, COLOR_PAIR(i + 1) | A_BOLD);
        
        mvwprintw(win, row, SCORE_COLUMN_POS, SCORE_FORMAT, p->score);
        mvwprintw(win, row, VALID_COLUMN_POS, MOVES_FORMAT, p->valid_moves);
        mvwprintw(win, row, INVALID_COLUMN_POS, MOVES_FORMAT, p->invalid_moves);
    }
}

void draw_matrix(WINDOW* win, const game_state_t* game_state_ptr) {
    werase(win);
    box(win, 0, 0);
    
    // Draw matrix
    for (unsigned int i = 0; i < game_state_ptr->height; i++) {
        for (unsigned int j = 0; j < game_state_ptr->width; j++) {
            int value = game_state_ptr->board_data[i * game_state_ptr->width + j];
            int row = (int)i + 1;
            int col = (int)j * DEFAULT_MATRIX_CELL_WIDTH + WINDOW_BORDER_SIZE;
            
            // Check if any player is currently at this position
            bool player_here = false;
            int current_player = -1;
            for (unsigned int p = 0; p < game_state_ptr->player_count && p < MAX_PLAYERS; p++) {
                if (game_state_ptr->players[p].x_coord == j && game_state_ptr->players[p].y_coord == i) {
                    player_here = true;
                    current_player = (int)p;
                    break;
                }
            }
            
            if (player_here) {
                // Show "P" for current player position
                if (has_colors()) {
                    wattron(win, COLOR_PAIR(current_player + 1) | A_BOLD);
                    mvwprintw(win, row, col, PLAYER_MARKER_CHAR);
                    wattroff(win, COLOR_PAIR(current_player + 1) | A_BOLD);
                } else {
                    mvwprintw(win, row, col, PLAYER_MARKER_CHAR);
                }
            } else if (value > 0) {
                // White for positive values
                mvwprintw(win, row, col, NUMBER_FORMAT, value);
            } else {
                // Colored for values <= 0 (player territories)
                int player_idx = -value;  // 0 -> player 0, -1 -> player 1, etc.
                if (player_idx < MAX_PLAYERS && has_colors()) {
                    wattron(win, COLOR_PAIR(player_idx + 1) | A_BOLD);
                    mvwprintw(win, row, col, NUMBER_FORMAT, value);
                    wattroff(win, COLOR_PAIR(player_idx + 1) | A_BOLD);
                } else {
                    mvwprintw(win, row, col, NUMBER_FORMAT, value);
                }
            }
        }
    }
}

void handle_adaptive_resize(int *scr_h, int *scr_w, int game_width, int game_height, 
                           const game_state_t* gs_ptr, WINDOW **leaderboard_win, WINDOW **matrix_win) {
    // Update ncurses after resize
    endwin();
    refresh();
    clear();
    
    // Get new screen dimensions
    getmaxyx(stdscr, *scr_h, *scr_w);
    
    // Use default, fixed sizes
    int leaderboard_w = DEFAULT_LEADERBOARD_WIDTH;
    int matrix_cell_width = DEFAULT_MATRIX_CELL_WIDTH;
    
    // Recalculate window positions
    int leaderboard_h = (int)gs_ptr->player_count + 3;
    int leaderboard_y = LEADERBOARD_START_Y;
    int leaderboard_x = (*scr_w - leaderboard_w) / 2;
    if (leaderboard_x < 0) leaderboard_x = 0;
    
    int matrix_h = game_height + WINDOW_BORDER_SIZE;
    int matrix_w = game_width * matrix_cell_width + WINDOW_BORDER_SIZE;
    int matrix_y = leaderboard_y + leaderboard_h + LEADERBOARD_MATRIX_SPACING;
    int matrix_x = (*scr_w - matrix_w) / 2;
    if (matrix_x < 0) matrix_x = 0;
    
    // Ensure everything fits on screen
    if (matrix_y + matrix_h > *scr_h) {
        matrix_y = *scr_h - matrix_h - 1;
        if (matrix_y < leaderboard_y + leaderboard_h) {
            matrix_y = leaderboard_y + leaderboard_h;
        }
    }
    
    // Delete old windows
    delwin(*leaderboard_win);
    delwin(*matrix_win);
    
    // Create new windows with fixed sizes
    *leaderboard_win = newwin(leaderboard_h, leaderboard_w, leaderboard_y, leaderboard_x);
    *matrix_win = newwin(matrix_h, matrix_w, matrix_y, matrix_x);
}
