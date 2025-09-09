# Minimal Makefile to build the view app with ncurses

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -I./shared_memory -I./utils -I./master -g
NCURSES_LIB := -lncurses

# Valgrind configuration
VALGRIND := valgrind
VG_FLAGS := --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1
TERM ?= xterm-256color

BIN_DIR := bin
VIEW_SRC := view/view.c
MASTER_SRC := master/master.c
PLAYER_SRC := player/player1.c
PLAYER_LIB_SRC := player/player_lib.c
PLAYER2_SRC := player/player2.c
SHM_SRC := shared_memory/shm.c
SHM_OBJ := shared_memory/shm.o
GAME_CONFIG_SRC := utils/game_config.c
GAME_CONFIG_OBJ := utils/game_config.o
SOCKET_UTILS_SRC := utils/socket_utils.c
SOCKET_UTILS_OBJ := utils/socket_utils.o
SETUP_SRC := master/setup.c
SETUP_OBJ := master/setup.o

TARGETS := $(BIN_DIR)/view $(BIN_DIR)/master $(BIN_DIR)/player $(BIN_DIR)/player2

.PHONY: all clean valgrind-test pvs-analysis full-analysis pvs-test valgrind-test-1p valgrind-test-2-same-p valgrind-test-2-mixed-p

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
$(BIN_DIR)/player: $(PLAYER_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(PLAYER_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ)

# Jugador variant 2
$(BIN_DIR)/player2: $(PLAYER2_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(PLAYER2_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ)

clean:
	rm -rf $(BIN_DIR) *.o $(SHM_OBJ) $(GAME_CONFIG_OBJ) $(SOCKET_UTILS_OBJ) $(SETUP_OBJ) PVS-Studio.log report.tasks compile_commands.json strace_out

# Valgrind memory leak detection scenarios
# Note: These tests clean old logs before running.

valgrind-test-1p: all
	@echo "--- Running Valgrind (master + view + 1 player) ---"
	@rm -f valgrind_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_1p_%p.log \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 1 -v $(BIN_DIR)/view -p $(BIN_DIR)/player
	@echo "Valgrind logs generated (valgrind_1p_*.log)."

valgrind-test-2-same-p: all
	@echo "--- Running Valgrind (master + view + 2 identical players) ---"
	@rm -f valgrind_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_2p_same_%p.log \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 2 -v $(BIN_DIR)/view -p $(BIN_DIR)/player
	@echo "Valgrind logs generated (valgrind_2p_same_*.log)."

# Valgrind test with 2 different players
# NOTE: This assumes your master program can accept multiple '-p' arguments.
valgrind-test-2-mixed-p: all
	@echo "--- Running Valgrind (master + view + 2 different players) ---"
	@rm -f valgrind_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_2p_mixed_%p.log \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 2 -v $(BIN_DIR)/view -p $(BIN_DIR)/player -p $(BIN_DIR)/player2
	@echo "Valgrind logs generated (valgrind_2p_mixed_*.log)."

valgrind-test: valgrind-test-1p valgrind-test-2-same-p valgrind-test-2-mixed-p
	@echo "\n--- All Valgrind tests completed. ---"
	@echo "Tip: grep -H 'LEAK SUMMARY' valgrind_*.log"

# PVS-Studio analysis using bear (recommended for this environment)
pvs-test:
	@echo "Running PVS-Studio Analysis with bear..."
	@$(MAKE) clean && \
	bear -- $(MAKE) all && \
	pvs-studio-analyzer analyze -o PVS-Studio.log && \
	plog-converter -a '64:1,2,3;GA:1,2,3;OP:1,2,3' -t tasklist -o report.tasks PVS-Studio.log
	@echo "PVS-Studio analysis complete. Report generated at report.tasks"

# Default runtime configuration for `make run` (override on the command line)
w ?= 10 # width
h ?= 10 # height
d ?= 200 # delay
t ?= 2 # timeout
RUN_SEED ?= 1
v ?= $(BIN_DIR)/view
# p: space-separated list of player binaries, e.g. p="$(BIN_DIR)/player $(BIN_DIR)/player2"
p ?= $(BIN_DIR)/player
# Pass a single -p followed by all player paths so the master's parsing (which
# expects -p followed by N player args) works correctly.
player_flags := -p $(p)

.PHONY: run
run: all
	@echo "Running master with view and players: $(p)"
	TERM=$(TERM) $(BIN_DIR)/master -w $(w) -h $(h) -d $(d) -t $(t) -s $(RUN_SEED) -v $(v) $(player_flags)

