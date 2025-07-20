#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include "chunk.h"
#include "value.h"

#define MAX_STACK_SIZE 256

typedef struct {
    Chunk *chunk;
    uint8_t *instruction_pointer;
    Value stack[MAX_STACK_SIZE];
    Value *stack_pointer;  // Points to the top element + 1
} VirtualMachine;

typedef enum { INTERPRET_OK, INTERPRET_COMPILE_ERROR, INTERPRET_RUNTIME_ERROR } InterpretResult;

void init_virtual_machine();
void free_virtual_machine();

InterpretResult interpret(const char *source_code);
void push(Value value);
Value pop();

#endif
