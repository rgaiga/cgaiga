#include "chunk.h"

#include <stdlib.h>

#include "memory.h"
#include "value.h"

void init_chunk(Chunk *chunk) {
    chunk->size = 0;
    chunk->capacity = 0;
    chunk->code = NULL;

    chunk->lines = NULL;

    init_value_array(&chunk->constants);
}

void free_chunk(Chunk *chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    free_value_array(&chunk->constants);
    init_chunk(chunk);
}

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

void write_constant(Chunk *chunk, Value value, int line) {
    // Use OP_CONSTANT if chunk has less than 255 constants.
    if (chunk->constants.size < 255) {
        write_chunk(chunk, OP_CONSTANT, line);

        write_value_array(&chunk->constants, value);
        int constant_index = chunk->constants.size - 1;
        write_chunk(chunk, constant_index, line);

        return;
    }

    // TODO: OP_CONSTANT_LONG
}
