// Simple ncurses-based view that waits for the master to signal, renders, and acknowledges.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../utils/game_state.h"
#include "../utils/game_sync.h"

// --- Drawing helpers ---
static void draw_board(WINDOW *win, const game_state_t* gs, const unsigned char* trail_owner) {
    werase(win);
    box(win, 0, 0);
    int col_width = 3;
    for (unsigned int i = 0; i < gs->height; i++) {
        for (unsigned int j = 0; j < gs->width; j++) {
            int value = gs->board_data[i * gs->width + j];
            int row = (int)i + 1;
            int col = (int)(j * col_width + 2);
            // If the cell was visited by a player, use that player's color
            if (trail_owner) {
                unsigned char owner = trail_owner[i * gs->width + j];
                if (owner > 0 && has_colors()) {
                    start_color();
                    init_pair(1, COLOR_RED,     COLOR_BLACK);
                    init_pair(2, COLOR_GREEN,   COLOR_BLACK);
                    init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
                    init_pair(4, COLOR_BLUE,    COLOR_BLACK);
                    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
                    init_pair(6, COLOR_CYAN,    COLOR_BLACK);
                    init_pair(7, COLOR_WHITE,   COLOR_BLACK);
                    init_pair(8, COLOR_RED,     COLOR_BLACK);
                    init_pair(9, COLOR_GREEN,   COLOR_BLACK);
                    wattron(win, COLOR_PAIR(owner) | A_BOLD);
                    mvwprintw(win, row, col, "%2d", value);
                    wattroff(win, COLOR_PAIR(owner) | A_BOLD);
                    continue;
                }
            }
            mvwprintw(win, row, col, "%2d", value);
        }
    }

    // Overlay players on top of the printed grid using colors and initials
    if (has_colors()) {
        start_color();
        // define some color pairs for up to 9 players
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_CYAN,    COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
        init_pair(8, COLOR_RED,     COLOR_BLACK);
        init_pair(9, COLOR_GREEN,   COLOR_BLACK);
    }
    for (unsigned int p = 0; p < gs->player_count; p++) {
        const player_t* pl = &gs->players[p];
        int y = (int)pl->y_coord;
        int x = (int)pl->x_coord;
        if (y >= 0 && y < (int)gs->height && x >= 0 && x < (int)gs->width) {
            int row = y + 1; // inside box
            int col = (int)(x * col_width + 2);
            // Choose display char: first letter of name or 'P'
            char ch = (pl->name[0] != '\0') ? pl->name[0] : 'P';
            if (has_colors()) wattron(win, COLOR_PAIR((int)((p % 9) + 1)) | A_BOLD);
            mvwprintw(win, row, col, " %c", ch);
            if (has_colors()) wattroff(win, COLOR_PAIR((int)((p % 9) + 1)) | A_BOLD);
        }
    }
}

static void draw_leaderboard(WINDOW* win, const game_state_t* gs) {
    werase(win);
    box(win, 0, 0);
    mvwprintw(win, 1, 2, "Name");
    mvwprintw(win, 1, 18, "Score");
    mvwprintw(win, 1, 28, "Val");
    mvwprintw(win, 1, 34, "Inv");

    for (unsigned int i = 0; i < gs->player_count; i++) {
        const player_t* p = &gs->players[i];
        int row = (int)i + 2; // start below header
        mvwprintw(win, row, 2, "%-14s", p->name);
        mvwprintw(win, row, 18, "%6u", p->score);
        mvwprintw(win, row, 28, "%4u", p->valid_moves);
    mvwprintw(win, row, 34, "%4u", p->invalid_moves);
    }
}

// Copy a snapshot of the state (including flexible array) into a local buffer
// No snapshot needed: the view reads directly from shared memory (read-only)

int main(void) {
    // Open shared memory objects directly to discover dynamic size.
    int gs_fd = shm_open("/game_state", O_RDONLY, 0);
    if (gs_fd == -1) {
        perror("view: shm_open(/game_state)");
        return 1;
    }
    int sync_fd = shm_open("/game_sync", O_RDWR, 0);
    if (sync_fd == -1) {
        perror("view: shm_open(/game_sync)");
        close(gs_fd);
        return 1;
    }

    // First, map only the header to read width/height.
    size_t header_size = sizeof(game_state_t);
    void* header_map = mmap(NULL, header_size, PROT_READ, MAP_SHARED, gs_fd, 0);
    if (header_map == MAP_FAILED) {
        perror("view: mmap header");
        close(gs_fd);
        close(sync_fd);
        return 1;
    }
    const game_state_t* gs_header = (const game_state_t*)header_map;
    unsigned int width = gs_header->width;
    unsigned int height = gs_header->height;
    unsigned int player_count = gs_header->player_count;
    munmap(header_map, header_size);

    // Re-map full state based on dimensions
    size_t gs_size = sizeof(game_state_t) + (size_t)width * (size_t)height * sizeof(int);
    void* gs_map = mmap(NULL, gs_size, PROT_READ, MAP_SHARED, gs_fd, 0);
    if (gs_map == MAP_FAILED) {
        perror("view: mmap full state");
        close(gs_fd);
        close(sync_fd);
        return 1;
    }
    const game_state_t* gs_ptr = (const game_state_t*)gs_map;

    // Map sync
    size_t sync_size = sizeof(game_sync_t);
    void* sync_map = mmap(NULL, sync_size, PROT_READ | PROT_WRITE, MAP_SHARED, sync_fd, 0);
    if (sync_map == MAP_FAILED) {
        perror("view: mmap sync");
        munmap(gs_map, gs_size);
        close(gs_fd);
        close(sync_fd);
        return 1;
    }
    game_sync_t* sync_ptr = (game_sync_t*)sync_map;

    // Init ncurses
    if (getenv("TERM") == NULL){
        setenv("TERM", "xterm-256color", 1);
    }

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    int scr_h, scr_w;
    getmaxyx(stdscr, scr_h, scr_w);

    // Windows layout
    int leaderboard_h = (int)player_count + 3; // border + header + rows
    int leaderboard_w = 42;
    int leaderboard_y = 1;
    int leaderboard_x = (scr_w - leaderboard_w) / 2;
    if (leaderboard_x < 0) leaderboard_x = 0;

    int matrix_h = (int)height + 2; // border
    int matrix_w = (int)(width * 3) + 2;
    int matrix_y = leaderboard_y + leaderboard_h + 1;
    int matrix_x = (scr_w - matrix_w) / 2;
    if (matrix_x < 0) matrix_x = 0;

    WINDOW *leaderboard_win = newwin(leaderboard_h, leaderboard_w, leaderboard_y, leaderboard_x);
    WINDOW *board_win = newwin(matrix_h, matrix_w, matrix_y, matrix_x);

    // Trail owner buffer: 0 means none, N (1..9) means owner is player index N-1
    size_t trail_sz = (size_t)width * (size_t)height;
    unsigned char* trail_owner = (unsigned char*)calloc(trail_sz, 1);
    if (!trail_owner) {
        endwin();
        munmap(sync_map, sync_size);
        munmap(gs_map, gs_size);
        close(gs_fd);
        close(sync_fd);
        fprintf(stderr, "view: out of memory for trail buffer\n");
        return 1;
    }

    // Main printing loop: block on master_to_view -> snapshot -> draw -> acknowledge
    for (;;) {
        if (sem_wait(&sync_ptr->master_to_view) != 0) {
            perror("view: sem_wait master_to_view");
            break;
        }

        // Update trail with current positions of players (persist across frames)
        for (unsigned int p = 0; p < gs_ptr->player_count; p++) {
            unsigned int x = gs_ptr->players[p].x_coord;
            unsigned int y = gs_ptr->players[p].y_coord;
            if (x < gs_ptr->width && y < gs_ptr->height) {
                trail_owner[y * gs_ptr->width + x] = (unsigned char)((p % 9) + 1);
            }
        }

        draw_leaderboard(leaderboard_win, gs_ptr);
        draw_board(board_win, gs_ptr, trail_owner);

        refresh();
        wrefresh(leaderboard_win);
        wrefresh(board_win);

        sem_post(&sync_ptr->view_to_master);

        if (gs_ptr->game_over) {
            break;
        }
    }

    // Cleanup ncurses and resources
    delwin(leaderboard_win);
    delwin(board_win);
    curs_set(1);
    endwin();

    free(trail_owner);
    munmap(sync_map, sync_size);
    munmap(gs_map, gs_size);
    close(gs_fd);
    close(sync_fd);
    return 0;
}