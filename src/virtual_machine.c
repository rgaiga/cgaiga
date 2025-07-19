#include "virtual_machine.h"

#include <stdio.h>

#include "chunk.h"
// #include "common.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"

VirtualMachine vm;

static void reset_stack() { vm.stack_pointer = vm.stack; }

void init_virtual_machine() { reset_stack(); }
void free_virtual_machine() {}

void push(Value value) {
    *vm.stack_pointer = value;
    vm.stack_pointer++;
};

Value pop() {
    vm.stack_pointer--;
    return *vm.stack_pointer;
}

static InterpretResult run() {
#define READ_BYTE() (*vm.instruction_pointer++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)     \
    do {                  \
        double b = pop(); \
        double a = pop(); \
        push(a op b);     \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stack_pointer; slot++) {
            printf("[ ");
            print_value(*slot);
            printf(" ]");
        }
        printf("\n");
        disassemble_instruction(vm.chunk, (int)(vm.instruction_pointer - vm.chunk->code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_ADD:
                BINARY_OP(+);
                break;
            case OP_SUBTRACT:
                BINARY_OP(-);
                break;
            case OP_MULTIPLY:
                BINARY_OP(*);
                break;
            case OP_DIVIDE:
                BINARY_OP(/);
                break;
            case OP_NEGATE: {
                push(-pop());
                break;
            }
            case OP_RETURN: {
                print_value(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char* source_code) {
    Chunk chunk;
    init_chunk(&chunk);

    if (!compile(source_code, &chunk)) {
        free_chunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.instruction_pointer = vm.chunk->code;

    InterpretResult result = run();

    free_chunk(&chunk);
    return result;
}
