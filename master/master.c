#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>  
#include "../utils/game_state.h"
#include "../utils/game_sync.h"
#include "../shared_memory/shm.h"
#include "../utils/game_config.h"
#include "master.h"

extern char **environ;


int main(int argc, char *argv[]) {
    printf("Proceso Máster iniciado.\n");

    game_config_t config = parse_arguments(argc, argv);

    srand(config.seed);
    
    int game_state_fd;
    int game_sync_fd;
    game_state_t* game_state_ptr;
    game_sync_t* game_sync_ptr;

    const size_t shm_state_size = sizeof(game_state_t) + ((size_t)config.width * (size_t)config.height * sizeof(int)); 
    const size_t shm_sync_size = sizeof(game_sync_t);

    game_state_ptr = (game_state_t*)shm_init("/game_state", shm_state_size, &game_state_fd, O_RDWR);
    
    game_sync_ptr = (game_sync_t*)shm_init("/game_sync", shm_sync_size, &game_sync_fd, O_RDWR);
    
    init_game_state(config, game_state_ptr);
    
    init_game_sync(game_sync_ptr, config.player_count);
    
    init_players(config, game_state_ptr);

    int rfd[config.player_count];
    create_players(rfd, config.player_count, config.width, config.height, config , game_state_ptr);

    unsigned int start = 0;
    unsigned int remaining_players = config.player_count;
    
    while(!game_state_ptr->game_over){
        fd_set readfds;
        printf("Ciclo máster\n"); 
        
        // Configurar file descriptors para select
        int max_fd = setup_fd_set(&readfds, rfd, config.player_count);
        
        // Esperar actividad de jugadores con timeout
        int ready = wait_for_player_activity(max_fd, &readfds, config.timeout);
        if (ready <= 0) {
            game_state_ptr->game_over = true;
            break;
        }

        // Procesar input de jugadores de forma sincronizada
        sem_wait(&game_sync_ptr->master_mutex);
        handle_player_input(rfd, config.player_count, &readfds, 
                           game_state_ptr, game_sync_ptr, config, 
                           &remaining_players, start);
        sem_post(&game_sync_ptr->master_mutex);
        
        // Verificar si quedan jugadores activos
        if (remaining_players == 0) {
            game_state_ptr->game_over = true;
        }
        
        start = (start + 1) % config.player_count;
    }

    // Cerrar pipes de comunicación con jugadores
    close_player_pipes(rfd, config.player_count);

    // Esperar por todos los procesos hijos para evitar zombies
    wait_for_children(game_state_ptr);

    // Cleanup synchronization
    destroy_game_sync(game_sync_ptr, config.player_count);

    // Limpiar recursos de memoria compartida
    shm_close(game_state_ptr, shm_state_size, game_state_fd);
    shm_destroy("/game_state");
    
    shm_close(game_sync_ptr, shm_sync_size, game_sync_fd);
    shm_destroy("/game_sync");
    
    printf("JOYA\n");
    return 0;
}


void init_board(game_state_t* game_state_ptr) {
    for (unsigned int i = 0; i < game_state_ptr->height * game_state_ptr->width; i++) {
        game_state_ptr->board_data[i] = rand() % 9 + 1;  // Random values 1-9
    }
}

void create_players(int rfd[], unsigned int player_count, unsigned int width, unsigned int height ,game_config_t config ,game_state_t* game_state_ptr) {
    for (unsigned int i = 0; i < player_count; i++) {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
       
        pid_t pid = fork();
        printf("pid es %d\n", pid);
     
         if (pid == 0) { // Child process
            close(pipe_fd[0]); // Close read end in child
            close(1); // Close stdout
            dup(pipe_fd[1]);
            close(pipe_fd[1]);
            
            // Convertir parámetros a strings
            char width_str[16];
            char height_str[16];
            snprintf(width_str, sizeof(width_str), "%u", width);
            snprintf(height_str, sizeof(height_str), "%u", height);
            execve(config.player_path[i], (char*[]){config.player_path[i], width_str, height_str, NULL}, environ);
            
            perror("execve");
            exit(EXIT_FAILURE);
        } else { // Parent process
            
            close(pipe_fd[1]); // Close read end in parent
            rfd[i] = pipe_fd[0]; // Store write end for communication
            game_state_ptr->players[i].pid = pid; // Store child PID
        }
      
    }
      
}

void init_game_state(game_config_t config , game_state_t *game_state_ptr) {
    game_state_ptr->width = config.width;
    game_state_ptr->height = config.height;
    game_state_ptr->player_count = config.player_count;
    game_state_ptr->game_over = false;

     // Initialize board with random values between 1 and 9
    init_board(game_state_ptr);
}


void init_players(game_config_t config, game_state_t* game_state_ptr) {
    for (unsigned int i = 0; i < config.player_count; i++) {
        // Extract just the executable name from the path for display
        const char* player_name = strrchr(config.player_path[i], '/');
        if (player_name) {
            player_name++; // Skip the '/'
        } else {
            player_name = config.player_path[i]; // No path separator found
        }
        
        strncpy(game_state_ptr->players[i].name, player_name, sizeof(game_state_ptr->players[i].name) - 1);
        game_state_ptr->players[i].name[sizeof(game_state_ptr->players[i].name) - 1] = '\0';
        
        game_state_ptr->players[i].score = 0;
        game_state_ptr->players[i].valid_moves = 0;
        game_state_ptr->players[i].invalid_moves = 0;
        game_state_ptr->players[i].x_coord = rand() % config.width; //checkear que no haya otro jugador
        game_state_ptr->players[i].y_coord = rand() % config.height;
        game_state_ptr->players[i].pid = 0; // To be set when player process starts
        game_state_ptr->players[i].is_blocked = false;
    }
}

// Función para inicializar los semáforos de game_sync_t
void init_game_sync(game_sync_t* sync_ptr, unsigned int player_count) {
    sem_init(&sync_ptr->master_to_view, 1, 0);  // máster avisa a vista
    sem_init(&sync_ptr->view_to_master, 1, 0);  // vista avisa a máster
    sem_init(&sync_ptr->master_mutex, 1, 1);  // mutex máster
    sem_init(&sync_ptr->game_state_mutex, 1, 1);  // mutex estado
    sem_init(&sync_ptr->readers_count_mutex, 1, 1);  // mutex para F
    sync_ptr->readers_count = 0;               // contador jugadores leyendo
    
    for (unsigned int i = 0; i < player_count && i < GAME_SYNC_MAX_PLAYERS; i++) {
        sem_init(&sync_ptr->player_semaphores[i], 1, 1);  // semáforo por jugador
    }
}

// Función para destruir los semáforos de game_sync_t
void destroy_game_sync(game_sync_t* sync_ptr, unsigned int player_count) {
    sem_destroy(&sync_ptr->master_to_view);
    sem_destroy(&sync_ptr->view_to_master);
    sem_destroy(&sync_ptr->master_mutex);
    sem_destroy(&sync_ptr->game_state_mutex);
    sem_destroy(&sync_ptr->readers_count_mutex);
    
    for (unsigned int i = 0; i < player_count && i < GAME_SYNC_MAX_PLAYERS; i++) {
        sem_destroy(&sync_ptr->player_semaphores[i]);
    }
}

// Función para cerrar los pipes de comunicación con los jugadores
void close_player_pipes(int rfd[], unsigned int player_count) {
    for (unsigned int i = 0; i < player_count; i++) {
        if (rfd[i] != -1) { // no se pq checkea que no sea -1
            close(rfd[i]);
            rfd[i] = -1;
        }
    }
}

// Función para esperar por todos los procesos hijos y evitar zombies
void wait_for_children(game_state_t* game_state_ptr) {
    for (unsigned int i = 0; i < game_state_ptr->player_count; i++) {
        if (game_state_ptr->players[i].pid > 0) {
            int status;
            waitpid(game_state_ptr->players[i].pid, &status, 0);
        }
    }
}

// Función para procesar el movimiento de un jugador
int process_player_move(game_state_t* game_state_ptr, unsigned int player_idx, char move) {
    // Incrementar contador de movimientos del jugador
    game_state_ptr->players[player_idx].valid_moves++;
    printf("Jugador %u se movió a %c\n", player_idx, move);
    // Bloquear al jugador después de 50 movimientos
    if (game_state_ptr->players[player_idx].valid_moves >= 50) {
        game_state_ptr->players[player_idx].is_blocked = true;
        printf("Jugador %u bloqueado después de %u movimientos\n", player_idx, game_state_ptr->players[player_idx].valid_moves);
        return 0; // Indicar que hubo cambios en el estado
    }
    
    return 0; // Indicar cambios (se incrementó el contador)
}

// Configura el fd_set para select y retorna el max_fd
int setup_fd_set(fd_set* readfds, int rfd[], unsigned int player_count) {
    FD_ZERO(readfds);
    int max_fd = -1;
    
    for (unsigned int i = 0; i < player_count; i++) {
        if (rfd[i] != -1) {
            FD_SET(rfd[i], readfds);
            if (rfd[i] > max_fd) max_fd = rfd[i];
        }
    }
    return max_fd;
}

// Espera actividad de jugadores con timeout
int wait_for_player_activity(int max_fd, fd_set* readfds, unsigned int timeout) {
    if (max_fd == -1) return -2; // No hay jugadores
    
    struct timeval tv = {timeout, 0};
    int ready = select(max_fd + 1, readfds, NULL, NULL, &tv);
    
    if (ready < 0) {
        perror("select");
        return -1;
    }
    if (ready == 0) {
        printf("Timeout alcanzado. Terminando juego por inactividad.\n");
        return 0;
    }
    return ready;
}

// Maneja la entrada de un jugador específico
void handle_single_player(int player_idx, int fd, game_state_t* game_state_ptr, 
                         game_sync_t* game_sync_ptr, game_config_t config,
                         int rfd[], unsigned int* remaining_players) {
    char c;
    ssize_t bytes_read = read(fd, &c, 1);
    
    if (bytes_read > 0) {
        sem_wait(&game_sync_ptr->game_state_mutex);
        int cambios = process_player_move(game_state_ptr, player_idx, c);
        sem_post(&game_sync_ptr->game_state_mutex);
        
        if (cambios) {
            sem_post(&game_sync_ptr->master_to_view);
            sem_wait(&game_sync_ptr->view_to_master);
            usleep(config.delay * 1000);
        }
        sem_post(&game_sync_ptr->player_semaphores[player_idx]);
        
    } else if (bytes_read == 0) { // EOF
        close(fd);
        rfd[player_idx] = -1;
        (*remaining_players)--;
        
    } else { // Error
        perror("read jugador");
        close(fd);
        rfd[player_idx] = -1;
        (*remaining_players)--;
    }
}

// Procesa todos los jugadores que tienen datos listos
void handle_player_input(int rfd[], unsigned int player_count, fd_set* readfds,
                        game_state_t* game_state_ptr, game_sync_t* game_sync_ptr,
                        game_config_t config, unsigned int* remaining_players, unsigned int start) {
    
    for (unsigned int count = 0; count < player_count; count++) {
        unsigned int player_idx = (start + count) % player_count;
        int fd = rfd[player_idx];
        
        if (fd != -1 && FD_ISSET(fd, readfds)) {
            handle_single_player(player_idx, fd, game_state_ptr, game_sync_ptr, 
                                config, rfd, remaining_players);
        }
    }
}