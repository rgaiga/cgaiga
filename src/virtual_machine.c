#include "virtual_machine.h"

#include <stdarg.h>
#include <stdio.h>

#include "compiler.h"
#include "debug.h"

VirtualMachine vm;

void init_virtual_machine();
void free_virtual_machine();
InterpretResult interpret(const char* source_code);

static InterpretResult run();
static void runtime_error(const char* format, ...);
static bool is_falsy(Value value);

static void push(Value value);
static Value pop();
static Value peek(int offset);
static void reset_stack();

// Initializes the Virtual Machine.
void init_virtual_machine() { reset_stack(); }

// Frees resources used by the Virtual Machine.
void free_virtual_machine() {}

// Interprets a given source code string.
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

// Executes bytecode instructions one by one.
static InterpretResult run() {
#define READ_INSTRUCTION() (*vm.instruction_pointer++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_INSTRUCTION()])
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

        switch (instruction = READ_INSTRUCTION()) {
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
            case OP_NOT:
                push(BOOLEAN_VALUE(is_falsy(pop())));
                break;
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

#undef READ_INSTRUCTION
#undef READ_CONSTANT
#undef BINARY_OP
}

// Reports a runtime error and resets the stack.
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

// Checks if the specified value is evaluated as falsy.
static bool is_falsy(Value value) {
    return IS_NIL(value) || (IS_BOOLEAN(value) && !AS_BOOLEAN(value));
}

// Pushes a value onto the stack.
static void push(Value value) {
    *vm.stack_pointer = value;
    vm.stack_pointer++;
};

// Pops a value from the stack.
static Value pop() {
    vm.stack_pointer--;
    return *vm.stack_pointer;
}

// Peeks at the stack at the specified offset and returns the value.
static Value peek(int offset) { return vm.stack_pointer[-1 - offset]; }

// Clears the stack.
static void reset_stack() { vm.stack_pointer = vm.stack; }
