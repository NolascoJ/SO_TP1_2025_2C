#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Hay que correr esto cuando se inicia el contenedor para poder correr ncurses
//  apt-get update && apt-get install -y libncurses-dev


void print_matrix(WINDOW *win, int yMax, int xMax) {
    int col_width = 3;
    for (int i = 1; i < yMax - 1; i++) {
        for (int j = 2; j < xMax - 1; j += col_width) {
            mvwprintw(win, i, j, "%d", rand() % 10);
        }
    }
}

int main(void) {
    initscr();            
    cbreak();             
    noecho();             
    curs_set(0);
    srand(time(NULL));    

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);

    // Leaderboard Window
    int leaderboard_h = 4;
    int leaderboard_w = 45;
    int leaderboard_y = 1;
    int leaderboard_x = (xMax - leaderboard_w) / 2;
    WINDOW *leaderboard_win = newwin(leaderboard_h, leaderboard_w, leaderboard_y, leaderboard_x);
    box(leaderboard_win, 0, 0);

    mvwprintw(leaderboard_win, 1, 2, "Name");
    mvwprintw(leaderboard_win, 2, 2, "player10.o");
    mvwprintw(leaderboard_win, 1, 15, "Score");
    mvwprintw(leaderboard_win, 2, 15, "157");
    mvwprintw(leaderboard_win, 1, 25, "ValRq");
    mvwprintw(leaderboard_win, 2, 25, "33");
    mvwprintw(leaderboard_win, 1, 35, "InvRq");
    mvwprintw(leaderboard_win, 2, 35, "4");


    // Matrix Window
    int matrix_h = yMax - leaderboard_h - 2;
    int matrix_w = xMax - 4;
    int matrix_y = leaderboard_y + leaderboard_h;
    int matrix_x = (xMax - matrix_w) / 2;
    WINDOW *win = newwin(matrix_h, matrix_w, matrix_y, matrix_x);
    box(win, 0, 0);
    
    int yWinMax, xWinMax;
    getmaxyx(win, yWinMax, xWinMax);
    print_matrix(win, yWinMax, xWinMax);

    refresh();
    wrefresh(leaderboard_win);
    wrefresh(win);

    int input = 0;
    while((input = getch()) != 'q');

    delwin(leaderboard_win);
    delwin(win);
    curs_set(1);
    endwin();
    return 0;
}