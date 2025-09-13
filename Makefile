# Minimal Makefile to build the view app with ncurses

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -I./shared_memory -I./common -I./master -g
NCURSES_LIB := -lncurses

# Valgrind configuration
VALGRIND := valgrind
VG_FLAGS := --leak-check=full --show-leak-kinds=all --track-origins=yes --trace-children=yes --error-exitcode=1
TERM ?= xterm-256color

# strace configuration
STRACE := strace
STRACE_FLAGS := -f -qq -tt -T -s 128

BIN_DIR := bin
VIEW_SRC := view/view.c
MASTER_SRC := master/master.c
PLAYER_SRC := player/player1.c
PLAYER_LIB_SRC := player/player_lib.c
PLAYER2_SRC := player/player2.c
PLAYER3_SRC := player/player3.c
SHM_SRC := shared_memory/shm.c
SHM_OBJ := shared_memory/shm.o
GAME_CONFIG_SRC := common/game_config.c
GAME_CONFIG_OBJ := common/game_config.o
SELECT_COMMON_SRC := common/select_utils.c
SELECT_COMMON_OBJ := common/select_utils.o
SETUP_SRC := master/setup.c
SETUP_OBJ := master/setup.o

TARGETS := $(BIN_DIR)/view $(BIN_DIR)/master $(BIN_DIR)/player $(BIN_DIR)/player2 $(BIN_DIR)/player3

.PHONY: all clean valgrind-test pvs-analysis full-analysis pvs-test valgrind-test-1p valgrind-test-2-same-p valgrind-test-2-mixed-p \
valgrind-1p-view valgrind-1p-noview valgrind-2p-same-view valgrind-2p-mixed-view \
valgrind-3p-mixed-view valgrind-3p-mixed-noview valgrind-maxp-9-noview \
valgrind-large-board-50x50-2p-noview valgrind-timeout-fast-2p-view valgrind-timeout-fast-2p-noview \
valgrind-delay-high-view valgrind-2p-mixed-seed1 valgrind-2p-mixed-seed2 valgrind-2p-mixed-seed3 \
valgrind-2p-mixed-seeds valgrind-no-view-2p valgrind-suite valgrind \
strace-1p strace-2p-same strace-2p-mixed strace-3p-mixed-noview \
strace-summary-1p strace-summary-2p-same strace-summary-2p-mixed strace-summary-3p-mixed-noview \
strace-suite

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $@

# Compilar el objeto de la utilería de memoria compartida
$(SHM_OBJ): $(SHM_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar el objeto de game_config
$(GAME_CONFIG_OBJ): $(GAME_CONFIG_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar el objeto de select_common
$(SELECT_COMMON_OBJ): $(SELECT_COMMON_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar el objeto de setup
$(SETUP_OBJ): $(SETUP_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Vista (necesita ncurses)
$(BIN_DIR)/view: $(VIEW_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(VIEW_SRC) $(SHM_OBJ) $(NCURSES_LIB)

# Máster
$(BIN_DIR)/master: $(MASTER_SRC) $(SETUP_OBJ) $(SHM_OBJ) $(GAME_CONFIG_OBJ) $(SELECT_COMMON_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(MASTER_SRC) $(SETUP_OBJ) $(SHM_OBJ) $(GAME_CONFIG_OBJ) $(SELECT_COMMON_OBJ)

# Jugador
$(BIN_DIR)/player: $(PLAYER_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(PLAYER_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ)

# Jugador variant 2
$(BIN_DIR)/player2: $(PLAYER2_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(PLAYER2_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ)

# Jugador variant 3
$(BIN_DIR)/player3: $(PLAYER3_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(PLAYER3_SRC) $(PLAYER_LIB_SRC) $(SHM_OBJ)

clean:
	rm -rf $(BIN_DIR) *.o $(SHM_OBJ) $(GAME_CONFIG_OBJ) $(SELECT_COMMON_OBJ) $(SETUP_OBJ) PVS-Studio.log report.tasks compile_commands.json strace_out

# Valgrind memory leak detection scenarios
# Each target cleans only its own log prefix before running.

valgrind-test-1p: all
	@echo "--- Running Valgrind (master + view + 1 player) ---"
	@rm -f valgrind_1p_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_1p_%p.log \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 1 -v $(BIN_DIR)/view -p $(BIN_DIR)/player
	@echo "Valgrind logs generated (valgrind_1p_*.log)."

valgrind-test-2-same-p: all
	@echo "--- Running Valgrind (master + view + 2 identical players) ---"
	@rm -f valgrind_2p_same_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_2p_same_%p.log \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 2 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player
	@echo "Valgrind logs generated (valgrind_2p_same_*.log)."

# Valgrind test with 2 different players (single -p followed by both players)
valgrind-test-2-mixed-p: all
	@echo "--- Running Valgrind (master + view + 2 different players) ---"
	@rm -f valgrind_2p_mixed_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_2p_mixed_%p.log \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 2 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player2
	@echo "Valgrind logs generated (valgrind_2p_mixed_*.log)."

valgrind-test: valgrind-test-1p valgrind-test-2-same-p valgrind-test-2-mixed-p
	@echo "\n--- All Valgrind tests completed. ---"
	@echo "Tip: grep -H 'LEAK SUMMARY' valgrind_*.log"

# Additional extensive Valgrind scenarios
valgrind-1p-view: all
	@echo "--- Valgrind: 1 player, view on, small board ---"
	@rm -f valgrind_1p_view_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_1p_view_%p.log \
		$(BIN_DIR)/master -w 12 -h 12 -d 50 -t 3 -s 42 -v $(BIN_DIR)/view -p $(BIN_DIR)/player

valgrind-1p-noview: all
	@echo "--- Valgrind: 1 player, no view, small board ---"
	@rm -f valgrind_1p_noview_*.log
	$(VALGRIND) $(VG_FLAGS) --log-file=valgrind_1p_noview_%p.log \
		$(BIN_DIR)/master -w 12 -h 12 -d 1 -t 3 -s 43 -p $(BIN_DIR)/player

valgrind-2p-same-view: all
	@echo "--- Valgrind: 2 players same AI, view on ---"
	@rm -f valgrind_2p_same_view_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_2p_same_view_%p.log \
		$(BIN_DIR)/master -w 15 -h 15 -d 100 -t 4 -s 100 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player

valgrind-2p-mixed-view: all
	@echo "--- Valgrind: 2 players mixed AI, view on ---"
	@rm -f valgrind_2p_mixed_view_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_2p_mixed_view_%p.log \
		$(BIN_DIR)/master -w 15 -h 15 -d 100 -t 4 -s 101 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player2

valgrind-3p-mixed-view: all
	@echo "--- Valgrind: 3 players mixed AI, view on ---"
	@rm -f valgrind_3p_mixed_view_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_3p_mixed_view_%p.log \
		$(BIN_DIR)/master -w 18 -h 18 -d 120 -t 5 -s 202 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player2 $(BIN_DIR)/player3

valgrind-3p-mixed-noview: all
	@echo "--- Valgrind: 3 players mixed AI, no view ---"
	@rm -f valgrind_3p_mixed_noview_*.log
	$(VALGRIND) $(VG_FLAGS) --log-file=valgrind_3p_mixed_noview_%p.log \
		$(BIN_DIR)/master -w 18 -h 18 -d 1 -t 5 -s 203 -p $(BIN_DIR)/player $(BIN_DIR)/player2 $(BIN_DIR)/player3

valgrind-maxp-9-noview: all
	@echo "--- Valgrind: 9 players (max), no view ---"
	@rm -f valgrind_max_players_*.log
	$(VALGRIND) $(VG_FLAGS) --log-file=valgrind_max_players_%p.log \
		$(BIN_DIR)/master -w 20 -h 20 -d 1 -t 6 -s 300 -p \
		$(BIN_DIR)/player $(BIN_DIR)/player2 $(BIN_DIR)/player3 \
		$(BIN_DIR)/player $(BIN_DIR)/player2 $(BIN_DIR)/player3 \
		$(BIN_DIR)/player $(BIN_DIR)/player2 $(BIN_DIR)/player3

valgrind-large-board-50x50-2p-noview: all
	@echo "--- Valgrind: large board 50x50, 2 players, no view ---"
	@rm -f valgrind_large_board_*.log
	$(VALGRIND) $(VG_FLAGS) --log-file=valgrind_large_board_%p.log \
		$(BIN_DIR)/master -w 50 -h 50 -d 1 -t 8 -s 400 -p $(BIN_DIR)/player $(BIN_DIR)/player2

valgrind-timeout-fast-2p-view: all
	@echo "--- Valgrind: fast timeout (t=1), 2 players, view on ---"
	@rm -f valgrind_timeout_view_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_timeout_view_%p.log \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 1 -s 500 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player2

valgrind-timeout-fast-2p-noview: all
	@echo "--- Valgrind: fast timeout (t=1), 2 players, no view ---"
	@rm -f valgrind_timeout_*.log
	$(VALGRIND) $(VG_FLAGS) --log-file=valgrind_timeout_%p.log \
		$(BIN_DIR)/master -w 10 -h 10 -d 1 -t 1 -s 501 -p $(BIN_DIR)/player $(BIN_DIR)/player2

valgrind-delay-high-view: all
	@echo "--- Valgrind: high delay with view (d=500) ---"
	@rm -f valgrind_high_delay_*.log
	TERM=$(TERM) $(VALGRIND) $(VG_FLAGS) --log-file=valgrind_high_delay_%p.log \
		$(BIN_DIR)/master -w 12 -h 12 -d 500 -t 6 -s 600 -v $(BIN_DIR)/view -p $(BIN_DIR)/player

valgrind-no-view-2p: all
	@echo "--- Valgrind: 2 players mixed AI, no view ---"
	@rm -f valgrind_no_view_*.log
	$(VALGRIND) $(VG_FLAGS) --log-file=valgrind_no_view_%p.log \
		$(BIN_DIR)/master -w 15 -h 15 -d 1 -t 4 -s 700 -p $(BIN_DIR)/player $(BIN_DIR)/player2

valgrind-2p-mixed-seed1: all
	@rm -f valgrind_2p_mixed_seed1_*.log
	$(VALGRIND) $(VG_FLAGS) --log-file=valgrind_2p_mixed_seed1_%p.log \
		$(BIN_DIR)/master -w 14 -h 14 -d 1 -t 4 -s 11 -p $(BIN_DIR)/player $(BIN_DIR)/player2

valgrind-2p-mixed-seed2: all
	@rm -f valgrind_2p_mixed_seed2_*.log
	$(VALGRIND) $(VG_FLAGS) --log-file=valgrind_2p_mixed_seed2_%p.log \
		$(BIN_DIR)/master -w 14 -h 14 -d 1 -t 4 -s 22 -p $(BIN_DIR)/player $(BIN_DIR)/player2

valgrind-2p-mixed-seed3: all
	@rm -f valgrind_2p_mixed_seed3_*.log
	$(VALGRIND) $(VG_FLAGS) --log-file=valgrind_2p_mixed_seed3_%p.log \
		$(BIN_DIR)/master -w 14 -h 14 -d 1 -t 4 -s 33 -p $(BIN_DIR)/player $(BIN_DIR)/player2

valgrind-2p-mixed-seeds: valgrind-2p-mixed-seed1 valgrind-2p-mixed-seed2 valgrind-2p-mixed-seed3
	@echo "--- Completed 2p mixed seeds suite ---"

valgrind-suite: \
	valgrind-test-1p \
	valgrind-test-2-same-p \
	valgrind-test-2-mixed-p \
	valgrind-1p-view \
	valgrind-1p-noview \
	valgrind-2p-same-view \
	valgrind-2p-mixed-view \
	valgrind-3p-mixed-view \
	valgrind-3p-mixed-noview \
	valgrind-maxp-9-noview \
	valgrind-large-board-50x50-2p-noview \
	valgrind-timeout-fast-2p-view \
	valgrind-timeout-fast-2p-noview \
	valgrind-delay-high-view \
	valgrind-no-view-2p \
	valgrind-2p-mixed-seeds
	@echo "\n--- Full Valgrind suite completed. ---"

valgrind: valgrind-suite
	@echo "\n--- 'make valgrind' ran the full Valgrind suite ---"

# -----------------------------
# strace scenarios and summaries
# -----------------------------
strace-prepare:
	mkdir -p strace_out

strace-1p: all strace-prepare
	@echo "--- strace: master + view + 1 player (full trace) ---"
	rm -f strace_out/trace_1p* 
	TERM=$(TERM) $(STRACE) $(STRACE_FLAGS) -ff -o strace_out/trace_1p \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 1 -v $(BIN_DIR)/view -p $(BIN_DIR)/player
	@echo "Trace files: strace_out/trace_1p.*"

strace-2p-same: all strace-prepare
	@echo "--- strace: master + view + 2 identical players (full trace) ---"
	rm -f strace_out/trace_2p_same* 
	TERM=$(TERM) $(STRACE) $(STRACE_FLAGS) -ff -o strace_out/trace_2p_same \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 2 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player
	@echo "Trace files: strace_out/trace_2p_same.*"

strace-2p-mixed: all strace-prepare
	@echo "--- strace: master + view + 2 different players (full trace) ---"
	rm -f strace_out/trace_2p_mixed* 
	TERM=$(TERM) $(STRACE) $(STRACE_FLAGS) -ff -o strace_out/trace_2p_mixed \
		$(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 2 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player2
	@echo "Trace files: strace_out/trace_2p_mixed.*"

strace-3p-mixed-noview: all strace-prepare
	@echo "--- strace: 3 players mixed, no view (full trace) ---"
	rm -f strace_out/trace_3p_mixed_noview* 
	$(STRACE) $(STRACE_FLAGS) -ff -o strace_out/trace_3p_mixed_noview \
		$(BIN_DIR)/master -w 18 -h 18 -d 1 -t 5 -s 203 -p $(BIN_DIR)/player $(BIN_DIR)/player2 $(BIN_DIR)/player3
	@echo "Trace files: strace_out/trace_3p_mixed_noview.*"

# Syscall count summaries (-c): written to strace_out/*.summary.txt
strace-summary-1p: all strace-prepare
	@echo "--- strace -c: 1 player with view ---"
	TERM=$(TERM) $(STRACE) -f -c $(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 1 -v $(BIN_DIR)/view -p $(BIN_DIR)/player 2> strace_out/summary_1p.txt
	@echo "Summary: strace_out/summary_1p.txt"

strace-summary-2p-same: all strace-prepare
	@echo "--- strace -c: 2 identical players with view ---"
	TERM=$(TERM) $(STRACE) -f -c $(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 2 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player 2> strace_out/summary_2p_same.txt
	@echo "Summary: strace_out/summary_2p_same.txt"

strace-summary-2p-mixed: all strace-prepare
	@echo "--- strace -c: 2 mixed players with view ---"
	TERM=$(TERM) $(STRACE) -f -c $(BIN_DIR)/master -w 10 -h 10 -d 50 -t 2 -s 2 -v $(BIN_DIR)/view -p $(BIN_DIR)/player $(BIN_DIR)/player2 2> strace_out/summary_2p_mixed.txt
	@echo "Summary: strace_out/summary_2p_mixed.txt"

strace-summary-3p-mixed-noview: all strace-prepare
	@echo "--- strace -c: 3 mixed players, no view ---"
	$(STRACE) -f -c $(BIN_DIR)/master -w 18 -h 18 -d 1 -t 5 -s 203 -p $(BIN_DIR)/player $(BIN_DIR)/player2 $(BIN_DIR)/player3 2> strace_out/summary_3p_mixed_noview.txt
	@echo "Summary: strace_out/summary_3p_mixed_noview.txt"

strace-suite: \
	strace-summary-1p \
	strace-summary-2p-same \
	strace-summary-2p-mixed \
	strace-summary-3p-mixed-noview
	@echo "\n--- strace summary suite completed. Summaries in strace_out/*.txt ---"

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

