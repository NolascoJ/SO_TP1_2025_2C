# Minimal Makefile to build the view app with ncurses

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -I./shared_memory -I./utils -I./master
NCURSES_LIB := -lncurses

# Valgrind configuration
VALGRIND := valgrind
VG_FLAGS := --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1
TERM ?= xterm-256color

BIN_DIR := bin
VIEW_SRC := view/view.c
MASTER_SRC := master/master.c
PLAYER_SRC := player/player.c
SHM_SRC := shared_memory/shm.c
SHM_OBJ := shared_memory/shm.o
GAME_CONFIG_SRC := utils/game_config.c
GAME_CONFIG_OBJ := utils/game_config.o
SOCKET_UTILS_SRC := utils/socket_utils.c
SOCKET_UTILS_OBJ := utils/socket_utils.o
SETUP_SRC := master/setup.c
SETUP_OBJ := master/setup.o

TARGETS := $(BIN_DIR)/view $(BIN_DIR)/master $(BIN_DIR)/player

.PHONY: all clean valgrind-test pvs-analysis full-analysis

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $@

# Compilar el objeto de la utilería de memoria compartida
$(SHM_OBJ): $(SHM_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar el objeto de game_config
$(GAME_CONFIG_OBJ): $(GAME_CONFIG_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar el objeto de socket_utils
$(SOCKET_UTILS_OBJ): $(SOCKET_UTILS_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar el objeto de setup
$(SETUP_OBJ): $(SETUP_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Vista (necesita ncurses)
$(BIN_DIR)/view: $(VIEW_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(VIEW_SRC) $(SHM_OBJ) $(NCURSES_LIB)

# Máster
$(BIN_DIR)/master: $(MASTER_SRC) $(SETUP_OBJ) $(SHM_OBJ) $(GAME_CONFIG_OBJ) $(SOCKET_UTILS_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(MASTER_SRC) $(SETUP_OBJ) $(SHM_OBJ) $(GAME_CONFIG_OBJ) $(SOCKET_UTILS_OBJ)

# Jugador
$(BIN_DIR)/player: $(PLAYER_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(PLAYER_SRC) $(SHM_OBJ)

clean:
	rm -rf $(BIN_DIR) *.o $(SHM_OBJ) $(GAME_CONFIG_OBJ) $(SOCKET_UTILS_OBJ) $(SETUP_OBJ) PVS-Studio.log report.tasks compile_commands.json

# Valgrind memory leak detection (master with one player and the view)
valgrind-test: all
	@echo "Running Valgrind (master + view + 1 player)..."
	# Use per-process logs so child processes (like view) produce their own files
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_%p.log \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 1 -v $(BIN_DIR)/view -p $(BIN_DIR)/player
	@echo "Valgrind logs generated (one per PID):"
	@ls -1 valgrind_*.log || echo "No log files found"
	@echo "Tip: grep -H "LEAK SUMMARY" valgrind_*.log"

# PVS-Studio static analysis
pvs-analysis: clean
	@echo "Running PVS-Studio static analysis..."
	@echo "=== Capturing compiler invocations (trace) ==="
	pvs-studio-analyzer trace -o strace_out -- make clean all
	@echo "=== Running PVS-Studio analysis ==="
	pvs-studio-analyzer analyze -f strace_out -C gcc -o PVS-Studio.log
	@echo "=== Converting results to readable formats ==="
	plog-converter -a GA:1,2 -t tasklist -o report.tasks PVS-Studio.log
	plog-converter -a GA:1,2 -t text -o PVS-Studio.txt PVS-Studio.log
	@echo "=== Creating component-specific views (master/view/player) ==="
	@grep -E "/master/|\\bmaster\\.c\\b" PVS-Studio.txt > PVS-Studio-master.txt || true
	@grep -E "/view/|\\bview\\.c\\b"     PVS-Studio.txt > PVS-Studio-view.txt   || true
	@grep -E "/player/|\\bplayer\\.c\\b" PVS-Studio.txt > PVS-Studio-player.txt || true
	@echo "=== Analysis complete ==="
	@echo "Summary files: report.tasks, PVS-Studio.txt"
	@echo "Per-component: PVS-Studio-master.txt, PVS-Studio-view.txt, PVS-Studio-player.txt"

# Combined analysis (Valgrind + PVS-Studio)
full-analysis: pvs-analysis valgrind-test
	@echo "Full analysis completed!"
	@echo "PVS-Studio results: report.tasks"
	@echo "Valgrind results: valgrind_*.log"
