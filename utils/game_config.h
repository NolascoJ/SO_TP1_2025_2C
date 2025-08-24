#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <getopt.h>

#define DEFAULT_WIDTH 10
#define DEFAULT_HEIGHT 10
#define DEFAULT_DELAY 200 //ms
#define DEFAULT_TIMEOUT 10 //s
#define DEFAULT_VIEW_PATH NULL
#define MIN_PLAYERS 1
#define MAX_PLAYERS 9

typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int delay;
    unsigned int timeout;
    unsigned int seed;
    unsigned int player_count;
    char* view_path; 
    char* player_path[MAX_PLAYERS];
} game_config_t;

game_config_t parse_arguments(int argc, char* argv[]);
void print_usage(const char* exe);

#endif