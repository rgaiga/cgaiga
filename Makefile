CC = gcc
CFLAGS = -g -Wall -Wextra -std=c99

SRC = src/main.c src/chunk.c src/memory.c src/debug.c src/value.c \
      src/virtual_machine.c src/compiler.c src/scanner.c src/object.c src/hash_table.c
OBJ = $(SRC:src/%.c=build/%.o)

BIN_DIR = bin
BIN_NAME = gg
BIN = $(BIN_DIR)/$(BIN_NAME)

all: $(BIN)

$(BIN): $(OBJ) | $(BIN_DIR)
	$(CC) $(OBJ) -o $(BIN)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf build $(BIN_DIR)
