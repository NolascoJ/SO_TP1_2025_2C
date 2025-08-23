# Minimal Makefile to build the view app with ncurses

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -I./shared_memory -I./utils
NCURSES_LIB := -lncurses

BIN_DIR := bin
VIEW_SRC := view/view.c
MASTER_SRC := master/master.c
PLAYER_SRC := player/player.c
SHM_SRC := shared_memory/shm.c
SHM_OBJ := shared_memory/shm.o

TARGETS := $(BIN_DIR)/view $(BIN_DIR)/master $(BIN_DIR)/player

.PHONY: all clean valgrind-test pvs-analysis full-analysis

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $@

# Compilar el objeto de la utilería de memoria compartida
$(SHM_OBJ): $(SHM_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Vista (necesita ncurses)
$(BIN_DIR)/view: $(VIEW_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(VIEW_SRC) $(SHM_OBJ) $(NCURSES_LIB)

# Máster
$(BIN_DIR)/master: $(MASTER_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(MASTER_SRC) $(SHM_OBJ)

# Jugador
$(BIN_DIR)/player: $(PLAYER_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(PLAYER_SRC) $(SHM_OBJ)

clean:
	rm -rf $(BIN_DIR) *.o $(SHM_OBJ) PVS-Studio.log report.tasks compile_commands.json

# Valgrind memory leak detection
valgrind-test: all
	@echo "Running Valgrind tests..."
	@echo "=== Testing Master ==="
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind_master.log $(BIN_DIR)/master -h 5 -w 5 -p 1 &
	@sleep 2
	@pkill -f "$(BIN_DIR)/master" || true
	@echo "Master Valgrind log:"
	@cat valgrind_master.log || echo "No log file found"
	@echo ""
	@echo "=== Testing Player ==="
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind_player.log $(BIN_DIR)/player &
	@sleep 1
	@pkill -f "$(BIN_DIR)/player" || true
	@echo "Player Valgrind log:"
	@cat valgrind_player.log || echo "No log file found"
	@echo ""
	@echo "Valgrind tests completed. Check valgrind_*.log files for details."

# PVS-Studio static analysis
pvs-analysis: clean
	@echo "Running PVS-Studio static analysis..."
	@echo "=== Generating compilation database ==="
	pvs-studio-analyzer trace -- make all
	@echo "=== Running PVS-Studio analysis ==="
	pvs-studio-analyzer analyze -f strace_out --output-file PVS-Studio.log
	@echo "=== Converting results to readable format ==="
	plog-converter -a GA:1,2 -t tasklist -o report.tasks PVS-Studio.log
	@echo "=== Analysis complete ==="
	@echo "Check report.tasks for analysis results"
	@cat report.tasks || echo "No issues found or report generation failed"

# Combined analysis (Valgrind + PVS-Studio)
full-analysis: pvs-analysis valgrind-test
	@echo "Full analysis completed!"
	@echo "PVS-Studio results: report.tasks"
	@echo "Valgrind results: valgrind_*.log"
