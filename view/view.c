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

static volatile sig_atomic_t resize_needed = 0;

static void handle_resize(int sig) {
    (void)sig; // Unused parameter
    resize_needed = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "view2: expected 2 argame_state: <width> <height>\n");
        return 1;
    }
    
    int game_width = atoi(argv[1]);
    int game_height = atoi(argv[2]);

    // Map game state
    size_t game_state_size = sizeof(game_state_t) + (size_t)game_width * (size_t)game_height * sizeof(int);
    int game_state_fd;
    game_state_t* game_state_ptr = shm_open_and_map("/game_state", game_state_size, &game_state_fd, O_RDONLY);
    
    // Map sync
    size_t sync_size = sizeof(game_sync_t);
    int sync_fd;
    game_sync_t* sync_ptr = shm_open_and_map("/game_sync", sync_size, &sync_fd, O_RDWR);
    
    // Initialize ncurses
    if (getenv("TERM") == NULL) {
        setenv("TERM", "xterm-256color", 1);
    }

    SCREEN *screen = newterm(NULL, stdout, stdin);
    if (screen == NULL) {
        fprintf(stderr, "view: failed to initialize ncurses screen\n");
        shm_close(sync_ptr, sync_size, sync_fd);
        shm_close(game_state_ptr, game_state_size, game_state_fd);
        return 1;
    }
    set_term(screen);
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    
    init_player_colors();
    
    signal(SIGWINCH, handle_resize);
    
    int scr_h, scr_w;
    getmaxyx(stdscr, scr_h, scr_w);
    
    // Layout: leaderboard at top, matrix below
    int leaderboard_h = (int)game_state_ptr->player_count + 3;  // Header + actual players + borders
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
            handle_adaptive_resize(&scr_h, &scr_w, game_width, game_height, game_state_ptr, &leaderboard_win, &matrix_win);
        }
        
        // Draw components
        draw_leaderboard(leaderboard_win, game_state_ptr);
        draw_matrix(matrix_win, game_state_ptr);
        
        // Refresh screens
        refresh();
        wrefresh(leaderboard_win);
        wrefresh(matrix_win);

        if ( game_state_ptr->game_over) {
            is_over = true;
        }
        
        // Signal master that view is done
        sem_post(&sync_ptr->view_to_master);
        
        // Exit if game is over
  
    }while(!is_over);

    // Game is over, draw the final scoreboard
    draw_final_scoreboard(game_state_ptr);

    
    
    // Cleanup
    delwin(leaderboard_win);
    delwin(matrix_win);
    endwin();
    delscreen(screen);

    
    shm_close(sync_ptr, sync_size, sync_fd);
    shm_close(game_state_ptr, game_state_size, game_state_fd);
    
    return 0;
}

 void draw_final_scoreboard(const game_state_t* game_state_ptr) {
    // Sort player indices by score (desc). This preserves a stable mapping to original color pairs
    // and avoids ambiguous name/score matches when there are duplicates.
    typedef struct { unsigned int idx; unsigned int score; } player_idx_t;
    player_idx_t order[MAX_PLAYERS];
    for (unsigned int i = 0; i < game_state_ptr->player_count && i < MAX_PLAYERS; i++) {
        order[i].idx = i;
        order[i].score = game_state_ptr->players[i].score;
    }
    // Sort using file-scope comparator (score desc, index asc)
    qsort(order, game_state_ptr->player_count, sizeof(player_idx_t), compare_player_idx_desc);

    // Clear screen and prepare for final display
    clear();
    refresh();

    int scr_h, scr_w;
    getmaxyx(stdscr, scr_h, scr_w);

    // Calculate window size with extra space for the "Press any key" message
    int final_h = (int)game_state_ptr->player_count + 8; // More space for message
    int final_w = 60;
    int final_y = (scr_h - final_h) / 2;
    int final_x = (scr_w - final_w) / 2;

    WINDOW* final_win = newwin(final_h, final_w, final_y, final_x);
    box(final_win, 0, 0);

    // Title with color
    wattron(final_win, A_BOLD | COLOR_PAIR(PLAYER_COLOR_RED));
    mvwprintw(final_win, 1, (final_w - 11) / 2, "GAME OVER!");
    wattroff(final_win, A_BOLD | COLOR_PAIR(PLAYER_COLOR_RED));

    // Header with color
    wattron(final_win, A_BOLD | COLOR_PAIR(PLAYER_COLOR_ORANGE));
    mvwprintw(final_win, 3, 3, "Rank");
    mvwprintw(final_win, 3, 10, "Player");
    mvwprintw(final_win, 3, 35, "Final Score");
    wattroff(final_win, A_BOLD | COLOR_PAIR(PLAYER_COLOR_ORANGE));

    // Draw line under header
    mvwaddch(final_win, 4, 1, ACS_LTEE);
    for (int i = 2; i < final_w - 1; i++) {
        mvwaddch(final_win, 4, i, ACS_HLINE);
    }
    mvwaddch(final_win, 4, final_w - 1, ACS_RTEE);
    // Display players sorted by score, using each player's original color pair
    for (unsigned int i = 0; i < game_state_ptr->player_count; i++) {
        int row = (int)i + 5;
        unsigned int orig_idx_u = order[i].idx;
        int color_pair_num = (int)orig_idx_u + 1;

        if (has_colors()) {
            attr_t a = COLOR_PAIR(color_pair_num);
            if (color_pair_num <= PLAYER_COLOR_CYAN) a |= A_BOLD; // avoid bold on blue-bg pairs 8/9
            wattron(final_win, a);
            mvwprintw(final_win, row, 3, "#%d", (int)i + 1);
            mvwprintw(final_win, row, 10, "%-20s", game_state_ptr->players[orig_idx_u].name);
            mvwprintw(final_win, row, 35, "%u", game_state_ptr->players[orig_idx_u].score);
            wattroff(final_win, a);
        } else {
            mvwprintw(final_win, row, 3, "#%d", (int)i + 1);
            mvwprintw(final_win, row, 10, "%-20s", game_state_ptr->players[orig_idx_u].name);
            mvwprintw(final_win, row, 35, "%u", game_state_ptr->players[orig_idx_u].score);
        }
    }
    
    // Instructions at the bottom with color
    int instruction_row = 5 + (int)game_state_ptr->player_count + 1; // One row below last player
    wattron(final_win, A_BLINK | COLOR_PAIR(PLAYER_COLOR_GREEN));
    mvwprintw(final_win, instruction_row, (final_w - 25) / 2, "Press any key to exit...");
    wattroff(final_win, A_BLINK | COLOR_PAIR(PLAYER_COLOR_GREEN));
    
    wrefresh(final_win);

    // Wait for any key press to exit
    nodelay(stdscr, FALSE); // Make getch() wait for input
    getch();

    delwin(final_win);
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
        init_pair(PLAYER_COLOR_PURPLE,   COLOR_MAGENTA,  COLOR_BLACK);
        init_pair(PLAYER_COLOR_DARK_GREEN, COLOR_GREEN,   COLOR_BLACK);
        init_pair(PLAYER_COLOR_ORANGE,      COLOR_YELLOW,   COLOR_BLACK);
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
        
        // Display player name with color; avoid A_BOLD for pairs 8 and 9
        int pair_num = (int)i + 1;
        attr_t name_attr = COLOR_PAIR(pair_num);
        if (pair_num <= PLAYER_COLOR_CYAN) name_attr |= A_BOLD;
        wattron(win, name_attr);
        mvwprintw(win, row, NAME_COLUMN_POS, "%-*s", NAME_FIELD_WIDTH, p->name);
        wattroff(win, name_attr);
        
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
                    int cp = current_player + 1;
                    attr_t a = COLOR_PAIR(cp);
                    if (cp <= PLAYER_COLOR_CYAN) a |= A_BOLD;
                    wattron(win, a);
                    mvwprintw(win, row, col, PLAYER_MARKER_CHAR);
                    wattroff(win, a);
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
                    int cp2 = player_idx + 1;
                    attr_t a2 = COLOR_PAIR(cp2);
                    if (cp2 <= PLAYER_COLOR_CYAN) a2 |= A_BOLD;
                    wattron(win, a2);
                    mvwprintw(win, row, col, NUMBER_FORMAT, value);
                    wattroff(win, a2);
                } else {
                    mvwprintw(win, row, col, NUMBER_FORMAT, value);
                }
            }
        }
    }
}

void handle_adaptive_resize(int *scr_h, int *scr_w, int game_width, int game_height, 
                           const game_state_t* game_state_ptr, WINDOW **leaderboard_win, WINDOW **matrix_win) {
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
    int leaderboard_h = (int)game_state_ptr->player_count + 3;
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

int compare_player_idx_desc(const void* a, const void* b) {
    const player_idx_t* pa = (const player_idx_t*)a;
    const player_idx_t* pb = (const player_idx_t*)b;
    if (pb->score != pa->score) return (pb->score > pa->score) - (pb->score < pa->score);
    // tie-breaker: lower original index first for stability
    return (pa->idx > pb->idx) - (pa->idx < pb->idx);
}