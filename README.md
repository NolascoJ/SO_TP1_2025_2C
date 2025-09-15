# ChompChamps - Manual de Usuario y Arquitectura

## 1. Descripción General

**ChompChamps** es un juego multijugador snake-like implementado en C sobre un entorno POSIX. El juego consiste en una competencia entre varios jugadores (controlados por IA) para acumular el mayor puntaje posible moviéndose a través de un tablero 2D. Cada casilla del tablero tiene un valor numérico, y los jugadores suman puntos al moverse a una nueva casilla. Las casillas visitadas se consumen y no pueden ser reutilizadas.

El proyecto está diseñado como un sistema de múltiples procesos que se comunican a través de memoria compartida y se sincronizan mediante semáforos, siguiendo una arquitectura de `master` (orquestador), `players` (jugadores) y un `view` (vista opcional).


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
    3.  **Ciclo de Juego**: Espera su turno (usando su semáforo personal), implementa un bloqueo de lectura para tomar una "snapshot" del estado del juego, y la pasa a su función `getMove`.
    4.  **Comunicación**: Envía el movimiento decidido (un byte) al `master` a través de su `stdout`, que está redirigido a un pipe.
*   **Estrategias Individuales**: La lógica de decisión está encapsulada en la función `getMove`, que es única para cada ejecutable de jugador:
    *   **`greedy`**: Estrategia simple que siempre elige moverse a la casilla adyacente con el valor más alto.
    *   **`cluster`**: Estrategia avanzada basada en heurísticas que evalúa clusters de casillas de alto valor, calcula penalizaciones por aislamiento y optimiza el movimiento considerando el valor inmediato, proximidad a clusters y riesgo de encierro.

### 2.3. Proceso `view`

Es un visualizador opcional basado en `ncurses` que se ejecuta en un terminal.
*   **Conexión**: Se conecta a la memoria compartida en modo de solo lectura.
*   **Renderizado**: No actualiza la pantalla continuamente. Espera una señal del `master` (vía semáforo `master_to_view`) para redibujar el estado del juego.
*   **Interfaz**: Muestra dos componentes principales:
    1.  Un **marcador** (leaderboard) con el nombre, puntaje y estadísticas de movimientos de cada jugador.
    2.  Una **matriz** que representa el tablero, mostrando la posición de los jugadores y el valor de cada casilla.
*   **Sincronización**: Una vez que ha terminado de dibujar, avisa al `master` (vía semáforo `view_to_master`) para que el juego pueda continuar.
*   **Finalización**: Al final del juego, muestra el leaderboard con los resultados finales del juego.
### 2.4. Comunicación y Sincronización

*   **Memoria Compartida (IPC)**:
    *   `/game_state`: Almacena el estado completo del juego (tablero, dimensiones, datos de jugadores). Protegido por un esquema de lectores-escritores.
    *   `/game_sync`: Contiene los semáforos y mutex necesarios para la sincronización.
*   **Pipes Anónimos**: El `master` crea un pipe para cada `player`. El `stdout` de cada jugador se redirige al extremo de escritura del pipe, permitiendo al `master` leer los movimientos desde el extremo de lectura.
*   **Semáforos POSIX sin nombre**:
    *   `game_state_mutex`: Controla el acceso de escritura al estado del juego (solo para el `master`) y forma parte del bloqueo lectores-escritores.
    *   `player_semaphores[]`: Un semáforo por jugador. El `master` lo usa para permitirle a cada jugador mandar un nuevo movimiento, una vez haya procesado el que ya mandó.
    *   `readers_count_mutex`: Protege la variable `readers_count` que coordina el bloqueo de lectura.
    *   `master_to_view` y `view_to_master`: Sincronizan el dibujado de la interfaz gráfica entre el `master` y el `view`.

## 3. Estructura del Proyecto

```
ChompChamps/
├── bin/                    # Ejecutables compilados
│   ├── master             # Proceso orquestador principal
│   ├── view               # Visualizador con ncurses  
│   ├── greedy             # Jugador con estrategia greedy
│   └── cluster            # Jugador con estrategia de clusters
├── master/                # Código fuente del proceso master
│   ├── master.c           # Lógica principal del orquestador
│   ├── master.h           # Definiciones del master
│   ├── setup.c            # Funciones de inicialización del juego
│   └── setup.h            # Headers de configuración
├── player/                # Código fuente de los jugadores
│   ├── player_lib.c       # Lógica común (conexión, ciclo de juego)
│   ├── player_lib.h       # Headers de la librería común
│   ├── greedy_player.c    # Implementación estrategia Greedy
│   └── cluster_player.c   # Implementación estrategia Cluster-based
├── view/                  # Código fuente del visualizador
│   ├── view.c             # Lógica de renderizado con ncurses
│   └── view.h             # Headers del visualizador
├── shared_memory/         # Abstracción para memoria compartida POSIX
│   ├── shm.c              # Funciones de gestión de memoria compartida
│   └── shm.h              # Headers de memoria compartida
├── common/                # Módulos de utilidad
│   ├── game_config.c      # Parseo de argumentos de línea de comandos
│   ├── game_config.h      # Headers de configuración
│   ├── game_state.h       # Estructura game_state_t
│   ├── game_sync.h        # Estructura game_sync_t  
│   ├── select_utils.c     # Funciones helper para select()
│   └── select_utils.h     # Headers de select utilities
├── strace_out/            # Resultados de análisis strace
├── Dockerfile             # Configuración del entorno de desarrollo
├── Makefile               # Script de compilación y análisis
└── README.md              # Esta documentación
```

## 4. Manual de Uso

### 4.1. Requisitos del Sistema

#### Dependencias Obligatorias
- **Compilador**: `gcc` 
- **Make**: Para gestión de compilación
- **Docker**: Para entorno de desarrollo reproducible

#### Dependencias de Librerías  
- **libncurses-dev**: Para la interfaz gráfica del visualizador
- **valgrind**: Para análisis de memoria (incluido en Docker)
- **strace**: Para análisis de llamadas al sistema (incluido en Docker)
- **bear**: Para generar base de datos de compilación (incluido en Docker)

### 4.2. Compilación (Build)

El método recomendado para compilar el proyecto es utilizando la imagen de Docker proporcionada a continuación, que asegura un entorno consistente.


> **Nota sobre el Entorno Docker**

> El `Dockerfile` incluido en el proyecto construye una imagen personalizada que parte de la imagen base provista por la cátedra (`agodio/itba-so-multi-platform:3.0`). A esta imagen se le añaden herramientas adicionales como `ncurses` (necesaria para el `view`), `valgrind` (para análisis de memoria) y `bear` (para generar una base de datos de compilación, útil para ejecutar PVS-Studio en arquitecturas como Apple Silicon).

**Paso 1: Construir la imagen de Docker**

Primero, necesitas construir la imagen de Docker a partir del `Dockerfile`.

> **Nota sobre las credenciales de PVS-Studio:**
> El `Dockerfile` intentará configurar las credenciales de PVS-Studio si se proporcionan durante la compilación. Si no tienes una licencia de PVS-Studio, puedes omitir los siguientes argumentos y construir la imagen normalmente con `docker build -t chompchamps-env .`.
> 
> Si tienes credenciales, puedes pasarlas como argumentos de compilación de la siguiente manera:
> ```sh
> docker build -t chompchamps-env \
>   --build-arg PVS_STUDIO_USER="tu_usuario" \
>   --build-arg PVS_STUDIO_KEY="tu_clave" \
>   .
> ```

*Este comando crea una imagen llamada `chompchamps-env` que contiene todas las dependencias necesarias.*

**Paso 2: Entrar al contenedor de Docker**

Una vez construida la imagen, puedes iniciar un contenedor a partir de ella:
```sh
docker run --rm -v ${PWD}:/app -w /app --security-opt seccomp:unconfined -it chompchamps-env
```
*Este comando monta el directorio actual de tu proyecto dentro del contenedor en la carpeta `/app`.*

**Paso 3: Compilar el proyecto**

Una vez dentro de la terminal del contenedor, ejecuta `make` para compilar todos los binarios:
```sh
make all
```
Los ejecutables (`master`, `view`, `player`, `player2`, y `player3`) se crearán en el directorio `bin/`.

**Verificar la compilación:**
```bash
ls -la bin/
# Deberías ver los 4 ejecutables listados arriba
```

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
*   **-v `ruta_view`**: Ruta al ejecutable del visualizador. Si no se especifica, el juego corre sin interfaz. Por defecto: `Sin vista`.
*   **-p `jugadores`**: **Obligatorio**. Rutas a los ejecutables de los jugadores (de 1 a 9).

**Ejemplos de Uso:**

1. **Partida simple (1 jugador con visualizador):**
   ```bash
   ./bin/master -v ./bin/view -p ./bin/greedy
   ```

2. **Partida con 2 estrategias diferentes:**
   ```bash
   ./bin/master -v ./bin/view -p ./bin/greedy ./bin/cluster
   ```

3. **Partida en tablero grande (3 jugadores):**
   ```bash
   ./bin/master -w 20 -h 20 -v ./bin/view -p ./bin/greedy ./bin/cluster ./bin/greedy
   ```

4. **Partida rápida sin visualizador:**
   ```bash
   ./bin/master -t 3 -p ./bin/greedy ./bin/cluster
   ```

5. **Máximo número de jugadores (9):**
   ```bash
   ./bin/master -w 25 -h 25 -v ./bin/view -p \
     ./bin/greedy ./bin/cluster ./bin/greedy \
     ./bin/cluster ./bin/greedy ./bin/cluster \
     ./bin/greedy ./bin/cluster ./bin/greedy
   ```
    

### 4.4. Uso Avanzado del Makefile

El `Makefile` proporciona targets adicionales para facilitar la ejecución y el análisis del programa.

*   **Análisis de Fugas de Memoria con Valgrind**:
    Para ejecutar una suite completa de pruebas de memoria en diferentes escenarios:
    ```bash
    make valgrind
    ```
    
    Los resultados se guardarán en archivos `valgrind_*.log`. Para revisar los resultados:
    ```bash
    grep -H 'LEAK SUMMARY' valgrind_*.log
    ```

*   **Limpieza de Archivos**:
    ```bash
    make clean  # Elimina ejecutables y archivos objeto
    ```

## 5. Estrategias de Jugadores

### 5.1. Greedy Player (`./bin/greedy`)
**Algoritmo**: Se desplaza a la casilla adyacente de mayor valor

**Características**:
- Evalúa las 8 casillas adyacentes (incluye diagonales)- Selecciona siempre la de mayor valor inmediato
- Simple pero efectiva para acumulación rápida de puntos
- Vulnerable a quedar atrapada en zonas sin salida

**Uso recomendado**: Ideal para tableros pequeños o como baseline de comparación.

### 5.2. Cluster Player (`./bin/cluster`)
**Algoritmo**: Heurística avanzada basada en clusters de alto valor con penalización por aislamiento.

**Características**:
- **Valor inmediato**: Considera el valor de la casilla destino
- **Análisis de clusters**: Evalúa casillas de alto valor en un radio configurable
- **Penalización por aislamiento**: Evita posiciones con pocas opciones de escape
- **Modo solo**: Optimización especial cuando es el único jugador activo
- **Parámetros configurables**:
  - `ALPHA = 0.8`: Peso para valor de proximidad a clusters
  - `GAMMA = 1.0`: Coeficiente de penalización por aislamiento  
  - `CLUSTER_RADIUS = 3`: Radio de evaluación de clusters
  - `MIN_FREE_CELLS = 2`: Umbral para detección de aislamiento

**Uso recomendado**: Estrategia superior para tableros medianos y grandes, competencias multijugador.

## 6. Troubleshooting

### 6.1. Problemas Comunes

#### Error: "Permission denied" al ejecutar ejecutables
```bash
# Solución: Asegúrate de que los ejecutables tengan permisos
chmod +x bin/*
```

#### Error: "ncurses not found" durante compilación
```bash
# Solución: Instalar libncurses (en el contenedor ya está incluida)
apt-get update && apt-get install -y libncurses-dev
```

#### El visualizador no se muestra correctamente
```bash
# Verificar variable de entorno TERM
echo $TERM
# Si está vacía o incorrecta, configurar:
export TERM=xterm-256color
```

#### Procesos zombie o no terminan correctamente
```bash
# Matar todos los procesos relacionados
pkill -f "./bin/master"
pkill -f "./bin/view" 
pkill -f "./bin/greedy"
pkill -f "./bin/cluster"

# Limpiar memoria compartida si persiste
# (El master debería limpiarla automáticamente)
```

#### Errores de memoria compartida

Suelen ocurrir si se termina el programa con Ctrl + c o si termina inesperadamente.
```bash
# Verificar segmentos de memoria compartida existentes
ls /dev/shm/
# Si hay segmentos orphan, eliminarlos manualmente:
rm /dev/shm/game_state /dev/shm/game_sync
```

### 6.2. Limitaciones Conocidas

- **Máximo 9 jugadores**: Definido por `MAX_PLAYERS` en `game_state.h`
- **Tamaño mínimo de tablero**: 10x10 (validación en argumentos)
- **Dependencia de ncurses**: El view requiere un terminal compatible con ncurses
- **Plataforma**: Diseñado para sistemas POSIX (Linux/macOS)

## 7. Desarrollo y Contribución

### 7.1. Agregar una Nueva Estrategia

1. **Crear archivo fuente**: `player/mi_estrategia_player.c`
2. **Implementar función `getMove`**:
   ```c
   int getMove(player_t* player, game_state_t* state, int me) {
       // Tu lógica aquí
       // Retornar dirección: 0-7 (N, NE, E, SE, S, SW, W, NW)
   }
   ```
3. **Actualizar Makefile**: Agregar target de compilación
4. **Compilar**: `make mi_estrategia`

### 7.2. Estructura del Código

- **Separación de responsabilidades**: Lógica común en `player_lib.c`, estrategias individuales en archivos separados
- **Headers organizados**: Definiciones compartidas en `common/`
- **Abstracción de memoria**: Todo el manejo de memoria compartida en `shared_memory/`
- **Configuración centralizada**: Parseo de argumentos en `common/game_config.c`

### 7.3. Estándares de Código

- **Wall -Wextra**: Compilación con warnings habilitados
- **Documentación**: Comentarios en funciones públicas
- **PVS-Studio**: Análisis estático de código disponible con `make pvs-test`
