# Minimal Makefile to build the view app with ncurses

CC := gcc
CFLAGS := -std=c11 -Wall -Wextra
LDLIBS := -lncurses

SRC := ./view/view.c
BIN_DIR := bin
BIN := $(BIN_DIR)

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(BIN_DIR):
	mkdir -p $@

# Borra la carpeta bin por completo
clean:
	rm -rf $(BIN)
