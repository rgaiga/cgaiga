#ifndef CHUNK_H
#define CHUNK_H

#include "memory.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,

    OP_NIL,
    OP_TRUE,
    OP_FALSE,

    OP_EQUAL,
    OP_NOT,
    OP_GREATER,
    OP_LESS,

    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,

    OP_NEGATE,

    OP_POP,

    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,

    OP_PRINT,
    OP_RETURN,
} OpCode;

typedef struct {
    int size;
    int capacity;
    uint8_t *code;
    int *lines;
    ValueArray constants;
} Chunk;

void init_chunk(Chunk *chunk);
void free_chunk(Chunk *chunk);
void write_chunk(Chunk *chunk, uint8_t byte, int line);

int add_constant(Chunk *chunk, Value value);

#endif
