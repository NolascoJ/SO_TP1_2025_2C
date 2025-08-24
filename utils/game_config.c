#define _GNU_SOURCE
#include "game_config.h"


void print_usage(const char* exe) {
    printf("Usage: %s [OPTIONS] -p player1 [player2 ...]\n\n", exe);
    printf("Parameters:\n");
    printf("  -w width     Board width. Default and minimum: 10\n");
    printf("  -h height    Board height. Default and minimum: 10\n");
    printf("  -d delay     Milliseconds the master waits each time the state is printed. Default: 200\n");
    printf("  -t timeout   Timeout in seconds to receive valid movement requests. Default: 10\n");
    printf("  -s seed      Seed used for board generation. Default: time(NULL)\n");
    printf("  -v view      Path to the view binary. Default: No view\n");
    printf("  -p player1 [player2 ...]  Paths to player binaries. Minimum: 1, Maximum: 9\n");
    printf("  --help       Show this help message\n");
}

game_config_t parse_arguments(int argc, char* argv[]) {
    game_config_t config = {
        .width = DEFAULT_WIDTH,
        .height = DEFAULT_HEIGHT,
        .delay = DEFAULT_DELAY,
        .timeout = DEFAULT_TIMEOUT,
        .seed = time(NULL),
        .view_path = DEFAULT_VIEW_PATH,
        .player_count = 0,
        .player_path = {NULL}
    };

    int opt;
    bool players_path_set = false;
while((opt = getopt(argc, argv, "w:h:d:t:s:v:p:")) != -1) {
        switch (opt) {
            case 'w':
                config.width = (unsigned int)atoi(optarg);
               
                if (config.width < 10) {
                    fprintf(stderr, "Error: Width must be at least 10.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                config.height = (unsigned int)atoi(optarg);
               
                if (config.height < 10) {
                    fprintf(stderr, "Error: Height must be at least 10.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
                config.delay = (unsigned int)atoi(optarg);
                if (config.delay <= 0) {
                    fprintf(stderr, "Error: Delay must be positive.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 't':
                config.timeout = (unsigned int)atoi(optarg);
                if (config.timeout <= 0) {
                    fprintf(stderr, "Error: Timeout must be positive.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
                config.seed = (unsigned int)atoi(optarg);
                break;
            case 'v':
                config.view_path = optarg;
                break;
            case 'p':
                players_path_set = true;
                config.player_count = 0;
                for (int i = optind - 1; i < argc && config.player_count < 9; i++) {
                    if (argv[i][0] != '-') {
                        config.player_path[config.player_count] = argv[i];
                        config.player_count++;
                    } else {
                        break;
                    }
                }
                if (config.player_count < 1 || config.player_count > 9) {
                    fprintf(stderr, "Error: Player count must be between 1 and 9.\n");
                    exit(EXIT_FAILURE);
                }
                optind =optind - 1 + config.player_count;
                break;
            case '?':
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

  //chequeo de que haya algun jugador 
    if (!players_path_set) {
        fprintf(stderr, "Error: At least one player must be specified with -p.\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    return config;
}