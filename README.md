# ChompChamps - Manual de Usuario y Arquitectura

## 1. Descripción General

**ChompChamps** es un juego multijugador por turnos implementado en C sobre un entorno POSIX. El juego consiste en una competencia entre varios jugadores (controlados por IA) para acumular el mayor puntaje posible moviéndose a través de un tablero 2D. Cada casilla del tablero tiene un valor numérico, y los jugadores suman puntos al moverse a una nueva casilla. Las casillas visitadas se consumen y no pueden ser reutilizadas.

El proyecto está diseñado como un sistema de múltiples procesos que se comunican a través de memoria compartida y se sincronizan mediante semáforos, siguiendo una arquitectura de `master` (orquestador), `players` (agentes autónomos) y un `view` (visualizador opcional).

## 2. Arquitectura del Sistema

El sistema se compone de tres tipos de procesos principales que interactúan de la siguiente manera:

### 2.1. Proceso `master`

Es el núcleo del juego. Sus responsabilidades son:
*   **Inicialización**: Parsea los argumentos de línea de comandos, crea e inicializa los segmentos de memoria compartida (`/game_state` y `/game_sync`), genera el tablero con valores aleatorios y establece el estado inicial de los jugadores.
*   **Creación de Procesos**: Lanza los procesos `player` y, opcionalmente, el proceso `view`, estableciendo la comunicación con ellos.
*   **Orquestación del Juego**: Gestiona el bucle principal del juego, que sigue un esquema Round-Robin para dar turnos a los jugadores.
*   **Manejo de Movimientos**: Espera los movimientos de los jugadores a través de pipes. Cuando recibe un movimiento, lo valida, actualiza el estado del juego (posición del jugador, puntaje, estado del tablero) y gestiona las condiciones de bloqueo (si un jugador queda atrapado).
*   **Sincronización**: Utiliza semáforos para proteger el acceso a la memoria compartida y para coordinar las actualizaciones con el proceso `view`.
*   **Condiciones de Fin**: Declara el fin del juego si todos los jugadores quedan bloqueados o si se excede un tiempo de inactividad (`timeout`).
*   **Limpieza de Recursos**: Al finalizar la partida, se asegura de que todos los procesos hijos terminen y libera semáforos y segmentos de memoria compartida.

### 2.2. Procesos `player`

Cada jugador es un proceso independiente que ejecuta una estrategia de IA para moverse por el tablero.
*   **Arquitectura Común (`player_lib.c`)**: Todos los jugadores comparten una base de código que gestiona:
    1.  **Conexión**: Mapea la memoria compartida (`/game_state` en solo lectura y `/game_sync` en lectura/escritura).
    2.  **Identificación**: Descubre su propio índice (`me`) en el array de jugadores del estado global.
    3.  **Ciclo de Juego**: Espera su turno (usando su semáforo personal), implementa un bloqueo de lectura para tomar una "instantánea" del estado del juego, y la pasa a su función `getMove`.
    4.  **Comunicación**: Envía el movimiento decidido (un byte) al `master` a través de su `stdout`, que está redirigido a un pipe.
*   **Estrategias Individuales (`player1.c`, `player2.c`, `player3.c`)**: La lógica de decisión está encapsulada en la función `getMove`, que es única para cada ejecutable de jugador:
    *   `player1`: **Greedy**. Estrategia simple que siempre elige moverse a la casilla adyacente con el valor más alto.
    *   `player2`: **Heurística**. Calcula un "score" para cada movimiento posible, ponderando el valor de la casilla, el potencial de las casillas cercanas (clúster) y una penalización por riesgo de encierro.
    *   `player3`: **Territorial**. Una IA avanzada basada en una máquina de estados que busca calcular una "región segura", trazar un perímetro y finalmente rellenar el territorio capturado.

### 2.3. Proceso `view`

Es un visualizador opcional basado en `ncurses` que se ejecuta en un terminal.
*   **Conexión**: Se conecta a la memoria compartida en modo de solo lectura.
*   **Renderizado**: No actualiza la pantalla continuamente. Espera una señal del `master` (vía semáforo `master_to_view`) para redibujar el estado del juego.
*   **Interfaz**: Muestra dos componentes principales:
    1.  Un **marcador** (leaderboard) con el nombre, puntaje y estadísticas de movimientos de cada jugador.
    2.  Una **matriz** que representa el tablero, mostrando la posición de los jugadores y el valor de cada casilla.
*   **Sincronización**: Una vez que ha terminado de dibujar, avisa al `master` (vía semáforo `view_to_master`) para que el juego pueda continuar.
*   **Finalización**: Al final del juego, muestra una pantalla de "GAME OVER" con la clasificación final de los jugadores.

### 2.4. Comunicación y Sincronización

*   **Memoria Compartida (IPC)**:
    *   `/game_state`: Almacena el estado completo del juego (tablero, dimensiones, datos de jugadores). Protegido por un esquema de lectores-escritores.
    *   `/game_sync`: Contiene los semáforos y mutex necesarios para la sincronización.
*   **Pipes Anónimos**: El `master` crea un pipe para cada `player`. El `stdout` de cada jugador se redirige al extremo de escritura del pipe, permitiendo al `master` leer los movimientos desde el extremo de lectura.
*   **Semáforos POSIX sin nombre**:
    *   `game_state_mutex`: Controla el acceso de escritura al estado del juego (solo para el `master`) y forma parte del bloqueo lectores-escritores.
    *   `player_semaphores[]`: Un semáforo por jugador. El `master` lo usa para dar el turno a un jugador específico.
    *   `readers_count_mutex`: Protege la variable `readers_count` que coordina el bloqueo de lectura.
    *   `master_to_view` y `view_to_master`: Sincronizan el dibujado de la interfaz gráfica entre el `master` y el `view`.

## 3. Estructura del Proyecto

*   **`bin/`**: Directorio para los ejecutables compilados.
*   **`master/`**: Código fuente del proceso master.
    *   `master.c` y `master.h`: Lógica principal del orquestador.
    *   `setup.c` y `setup.h`: Funciones para inicializar el estado del juego.
*   **`player/`**: Código fuente de los jugadores.
    *   `player_lib.c` y `player_lib.h`: Lógica común compartida por todos los jugadores (conexión a memoria, ciclo de juego, etc.).
    *   `player1.c`: Implementación de la estrategia *Greedy*.
    *   `player2.c`: Implementación de la estrategia *Heurística*.
    *   `player3.c`: Implementación de la estrategia *Territorial*.
*   **`view/`**: Código fuente del visualizador.
    *   `view.c` y `view.h`: Lógica de renderizado con `ncurses`.
*   **`shared_memory/`**: Abstracción para la gestión de memoria compartida POSIX.
    *   `shm.c` y `shm.h`: Funciones para crear, mapear y destruir segmentos de memoria.
*   **`common/`**: Módulos de utilidad.
    *   `game_config.c` y `game_config.h`: Parseo de argumentos de línea de comandos.
    *   `game_state.h`: Definición de la estructura `game_state_t`.
    *   `game_sync.h`: Definición de la estructura `game_sync_t`.
    *   `select_common.c` y `select_common.h`: Funciones helper para `select()`.
*   **`Makefile`**: Script para compilación, ejecución y tareas de análisis (Valgrind, Strace).

## 4. Manual de Uso

### 4.1. Requisitos

*   `make`
*   `docker` (para el entorno de compilación y ejecución recomendado)
*   Librería `ncurses` (incluida en la imagen de Docker)
*   Un entorno compatible con POSIX (Linux, macOS).

### 4.2. Compilación (Build)

El método recomendado para compilar el proyecto es utilizando el contenedor de Docker proporcionado, que asegura un entorno consistente.


> El `Dockerfile` incluido en el proyecto construye una imagen personalizada que parte de la imagen base provista por la cátedra (`agodio/itba-so-multi-platform:3.0`). A esta imagen se le añaden herramientas adicionales como `ncurses` (necesaria para el `view`), `valgrind` (para análisis de memoria) y `bear` (para generar una base de datos de compilación, útil para ejecutar PVS-Studio en arquitecturas como Apple Silicon).

**Paso 1: Entrar al contenedor de Docker**

No se provee un script `run.sh`, por lo que debes entrar manualmente al contenedor. Ejecuta el siguiente comando en la raíz de tu proyecto:

```sh
docker run --rm -v ${PWD}:/app -w /app --security-opt secomp:unconfined -it agodio/itba-so-multi-platform:3.0
```
*Este comando monta el directorio actual de tu proyecto dentro del contenedor en la carpeta `/app`.*

**Paso 2: Compilar el proyecto**

Una vez dentro de la terminal del contenedor, ejecuta `make` para compilar todos los binarios:
```sh
make all
```
Los ejecutables (`master`, `view`, `player`, `player2`, y `player3`) se crearán en el directorio `bin/`.

### 4.3. Ejecución del Juego

El proceso `master` es el punto de entrada para iniciar una partida. Debe ejecutarse desde dentro del contenedor Docker.

**Sintaxis básica:**

`./bin/master [OPCIONES] -p <jugador1> [jugador2 ...]`

**Parámetros de Configuración:**

*   **-w `ancho`**: Ancho del tablero (mínimo 10). Por defecto: `10`.
*   **-h `alto`**: Alto del tablero (mínimo 10). Por defecto: `10`.
*   **-d `delay`**: Milisegundos que espera el `master` entre turnos si el `view` está activo. Por defecto: `200`.
*   **-t `timeout`**: Segundos de inactividad de jugadores antes de terminar el juego. Por defecto: `10`.
*   **-s `semilla`**: Semilla para la generación de números aleatorios del tablero. Por defecto: `time(NULL)`.
*   **-v `ruta_view`**: Ruta al ejecutable del visualizador. Si no se especifica, el juego corre sin interfaz. Por defecto: `Sin visualizador`.
*   **-p `jugadores`**: **Obligatorio**. Rutas a los ejecutables de los jugadores (de 1 a 9).

**Ejemplos de Uso:**

*   **Partida simple (1 jugador con visualizador):**
    
    `./bin/master -v ./bin/view -p ./bin/player`
    

*   **Partida compleja (3 jugadores distintos, tablero grande):**
    
    `./bin/master -w 20 -h 20 -v ./bin/view -p ./bin/player ./bin/player2 ./bin/player3`
    

*   **Partida rápida sin visualizador y con timeout corto:**
    
    `./bin/master -t 3 -p ./bin/player ./bin/player2`
    

### 4.4. Uso Avanzado del Makefile

El `Makefile` proporciona targets adicionales para facilitar la ejecución y el análisis del programa.

*   **Ejecución Rápida (`make run`)**:
    Puedes usar `make run` para lanzar una partida personalizando los parámetros directamente desde la línea de comandos.
    
    *   `w`: Ancho del tablero.
    *   `h`: Alto del tablero.
    *   `p`: Lista de jugadores (debe ir entre comillas dobles).

    **Ejemplo:**
    
    `make run w=15 h=15 p='"./bin/player2" "./bin/player3"'`
    
    Este comando, inicia una partida en un tablero de 15x15 con los jugadores `player2` y `player3`, usando el visualizador y los demás parámetros por defecto.

*   **Análisis de Fugas de Memoria con Valgrind**:
    Para ejecutar una suite completa de pruebas de memoria en diferentes escenarios, utiliza:
    
    `make valgrind`
    
    Los resultados se guardarán en archivos `valgrind_*.log` en la raíz del proyecto. Este comando es un alias para `make valgrind-suite`, que ejecuta una batería de tests exhaustivos.

*   **Análisis de Llamadas al Sistema con Strace**:
    Para generar resúmenes de las llamadas al sistema que realizan los procesos en diferentes escenarios:
    
    `make strace-suite`
    
    Los informes se generarán en el directorio `strace_out/`.
