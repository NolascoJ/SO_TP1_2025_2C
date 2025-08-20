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

.PHONY: all clean

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
	rm -rf $(BIN_DIR) *.o $(SHM_OBJ)
