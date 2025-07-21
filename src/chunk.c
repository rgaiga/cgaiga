#include "chunk.h"

#include <stdlib.h>

// Initializes the chunk.
void init_chunk(Chunk *chunk) {
    chunk->size = 0;
    chunk->capacity = 0;
    chunk->code = NULL;

    chunk->lines = NULL;

    init_value_array(&chunk->constants);
}

// Frees resources used by the chunk.
void free_chunk(Chunk *chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    free_value_array(&chunk->constants);
    init_chunk(chunk);
}

// Writes a byte to the specified chunk at the specified line.
void write_chunk(Chunk *chunk, uint8_t byte, int line) {
    if (chunk->capacity < chunk->size + 1) {
        int old_capacity = chunk->capacity;

        chunk->capacity = GROW_CAPACITY(old_capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
    }

    chunk->code[chunk->size] = byte;
    chunk->lines[chunk->size] = line;
    chunk->size++;
}

// Adds a constant value to the specified chunk.
int add_constant(Chunk *chunk, Value value) {
    write_value_array(&chunk->constants, value);
    return chunk->constants.size - 1;
}
