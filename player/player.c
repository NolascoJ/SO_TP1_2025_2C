#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#include "../utils/game_state.h"
#include "../utils/game_sync.h"
#include "../shared_memory/shm.h"

int main() {
    printf("Proceso Jugador iniciado.\n");

    // Declaración de variables para las memorias compartidas
    int game_state_fd, game_sync_fd;
    game_state_t* game_state_ptr;
    game_sync_t* game_sync_ptr;
    
    const size_t shm_state_size = sizeof(game_state_t) + (10 * 10 * sizeof(int));
    const size_t shm_sync_size = sizeof(game_sync_t);

    // 1. Conexión a las memorias compartidas (solo lectura del estado)
    game_state_ptr = (game_state_t*)shm_open_and_map("/game_state", shm_state_size, &game_state_fd, O_RDONLY);
    if (game_state_ptr == NULL) {
        perror("Fallo al abrir la memoria de estado.");
        return 1;
    }
    printf("Jugador: Conectado a /game_state.\n");

    // 2. Conexión a la memoria de semáforos (lectura y escritura)
    game_sync_ptr = (game_sync_t*)shm_open_and_map("/game_sync", shm_sync_size, &game_sync_fd, O_RDWR);
    if (game_sync_ptr == NULL) {
        perror("Fallo al abrir la memoria de sincronización.");
        shm_close(game_state_ptr, shm_state_size, game_state_fd);
        return 1;
    }
    printf("Jugador: Conectado a /game_sync.\n");
    
    // Aquí iría la lógica del jugador para leer el estado del juego y enviar movimientos

    // 3. Bucle principal del jugador
    while (!game_state_ptr->game_over) {
        // Lógica para decidir el próximo movimiento...
        sleep(1);
    }

    // 4. Limpiar recursos
    shm_close(game_state_ptr, shm_state_size, game_state_fd);
    shm_close(game_sync_ptr, shm_sync_size, game_sync_fd);

    printf("Jugador: Finalizado.\n");
    return 0;
}