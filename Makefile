CC = gcc
CFLAGS = -g -Wall -Wextra -std=c99

SRC = src/main.c src/chunk.c src/memory.c src/debug.c src/value.c \
      src/virtual_machine.c src/compiler.c src/scanner.c
OBJ = $(SRC:src/%.c=build/%.o)
BIN = cgaiga

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(BIN)
