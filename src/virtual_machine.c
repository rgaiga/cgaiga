#include "virtual_machine.h"

#include <stdarg.h>
#include <stdio.h>

#include "chunk.h"
// #include "common.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"

VirtualMachine vm;

static void reset_stack() { vm.stack_pointer = vm.stack; }

static void runtime_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.instruction_pointer - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    reset_stack();
}

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

static Value peek(int offset) { return vm.stack_pointer[-1 - offset]; }

static bool is_falsey(Value value) {
    return IS_NIL(value) || (IS_BOOLEAN(value) && !AS_BOOLEAN(value));
}

static InterpretResult run() {
#define READ_BYTE() (*vm.instruction_pointer++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(value_type, op)                         \
    do {                                                  \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtime_error("Operands must be numbers.");   \
            return INTERPRET_RUNTIME_ERROR;               \
        }                                                 \
                                                          \
        double b = AS_NUMBER(pop());                      \
        double a = AS_NUMBER(pop());                      \
        push(value_type(a op b));                         \
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
            case OP_NIL:
                push(NIL_VALUE);
                break;
            case OP_TRUE:
                push(BOOLEAN_VALUE(true));
                break;
            case OP_FALSE:
                push(BOOLEAN_VALUE(false));
                break;
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOLEAN_VALUE(values_equal(a, b)));
                break;
            }
            case OP_GREATER:
                BINARY_OP(BOOLEAN_VALUE, >);
                break;
            case OP_LESS:
                BINARY_OP(BOOLEAN_VALUE, <);
                break;
            case OP_ADD:
                BINARY_OP(NUMBER_VALUE, +);
                break;
            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VALUE, -);
                break;
            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VALUE, *);
                break;
            case OP_DIVIDE:
                BINARY_OP(NUMBER_VALUE, /);
                break;
            case OP_NOT:
                push(BOOLEAN_VALUE(is_falsey(pop())));
                break;
            case OP_NEGATE: {
                if (!IS_NUMBER(peek(0))) {
                    runtime_error("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VALUE(-AS_NUMBER(pop())));
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
