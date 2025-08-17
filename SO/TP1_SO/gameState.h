
typedef struct {
    char ?[16]; // Nombre del jugador
    unsigned int ?; // Puntaje
    unsigned int ?; // Cantidad de solicitudes de movimientos inválidas realizadas
    unsigned int ?; // Cantidad de solicitudes de movimientos válidas realizadas
    unsigned short ?, ?; // Coordenadas x e y en el tablero
    pid_t ?; // Identificador de proceso
    bool ?; // Indica si el jugador está bloqueado
    } XXX;

    typedef struct {
        unsigned short ?; // Ancho del tablero
        unsigned short ?; // Alto del tablero
        unsigned int ?; // Cantidad de jugadores
        XXX ?[9]; // Lista de jugadores
        bool ?; // Indica si el juego se ha terminado
        int ?[]; // Puntero al comienzo del tablero. fila-0, fila-1, ..., fila-n-1
        } YYY;

        typedef struct {
            sem_t A; // El máster le indica a la vista que hay cambios por imprimir
            sem_t B; // La vista le indica al máster que terminó de imprimir
            sem_t C; // Mutex para evitar inanición del máster al acceder al estado
            sem_t D; // Mutex para el estado del juego
            sem_t E; // Mutex para la siguiente variable
            unsigned int F; // Cantidad de jugadores leyendo el estado
            sem_t G[9]; // Le indican a cada jugador que puede enviar 1 movimiento
            } ZZZ;