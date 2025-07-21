#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum {
    PRECEDENCE_NONE,
    PRECEDENCE_ASSIGNMENT,  // =
    PRECEDENCE_OR,          // or
    PRECEDENCE_AND,         // and
    PRECEDENCE_EQUALITY,    // == !=
    PRECEDENCE_COMPARISON,  // < > <= >=
    PRECEDENCE_TERM,        // + -
    PRECEDENCE_FACTOR,      // * /
    PRECEDENCE_UNARY,       // ! -
    PRECEDENCE_CALL,        // . ()
    PRECEDENCE_PRIMARY
} Precedence;

typedef void (*ParseFunction)();

typedef struct {
    ParseFunction prefix;
    ParseFunction infix;
    Precedence precedence;
} ParseRule;

Parser parser;
Chunk *compiling_chunk;

static Chunk *current_chunk() { return compiling_chunk; }

static void error_at(Token *token, const char *message) {
    if (parser.panic_mode) return;
    parser.panic_mode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}

static void error(const char *message) { error_at(&parser.previous, message); }
static void error_at_current(const char *message) { error_at(&parser.current, message); }

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scan_token();
        if (parser.current.type != TOKEN_ERROR) break;

        error_at_current(parser.current.start);
    }
}

static void consume(TokenType type, const char *message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    error_at_current(message);
}

static void emit_byte(uint8_t byte) { write_chunk(current_chunk(), byte, parser.previous.line); }
static void emit_bytes(uint8_t byte1, uint8_t byte2) {
    emit_byte(byte1);
    emit_byte(byte2);
}

static void emit_return() { emit_byte(OP_RETURN); }

static uint8_t make_constant(Value value) {
    int constant = add_constant(current_chunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t)constant;
}
static void emit_constant(Value value) { emit_bytes(OP_CONSTANT, make_constant(value)); }

static void end_compiler() {
    emit_return();
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        disassemble_chunk(current_chunk(), "code");
    }
#endif
}

// Forward declarations
static void expression();
static ParseRule *get_rule(TokenType type);
static void parse_precedence(Precedence precedence);

static void parse_precedence(Precedence precedence) {
    advance();
    ParseFunction prefixRule = get_rule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression.");
        return;
    }

    prefixRule();

    while (precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        ParseFunction infixRule = get_rule(parser.previous.type)->infix;
        infixRule();
    }
}

static void binary() {
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = get_rule(operatorType);
    parse_precedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:
            emit_bytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emit_byte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emit_byte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emit_bytes(OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            emit_byte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emit_bytes(OP_GREATER, OP_NOT);
            break;
        case TOKEN_PLUS:
            emit_byte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emit_byte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emit_byte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emit_byte(OP_DIVIDE);
            break;
        default:
            return;  // Unreachable.
    }
}

static void literal() {
    switch (parser.previous.type) {
        case TOKEN_FALSE:
            emit_byte(OP_FALSE);
            break;
        case TOKEN_NIL:
            emit_byte(OP_NIL);
            break;
        case TOKEN_TRUE:
            emit_byte(OP_TRUE);
            break;
        default:
            return;  // Unreachable.
    }
}

static void expression() { parse_precedence(PRECEDENCE_ASSIGNMENT); }

static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PARENTHESIS, "Expect ')' after expression.");
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VALUE(value));
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    // Compile the operand.
    parse_precedence(PRECEDENCE_UNARY);

    // Emit the operator instruction.
    switch (operatorType) {
        case TOKEN_BANG:
            emit_byte(OP_NOT);
            break;
        case TOKEN_MINUS:
            emit_byte(OP_NEGATE);
            break;
        default:
            return;  // Unreachable.
    }
}

ParseRule rules[] = {
    [TOKEN_LEFT_PARENTHESIS] = {grouping, NULL, PRECEDENCE_NONE},
    [TOKEN_RIGHT_PARENTHESIS] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_DOT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_MINUS] = {unary, binary, PRECEDENCE_TERM},
    [TOKEN_PLUS] = {NULL, binary, PRECEDENCE_TERM},
    [TOKEN_SEMICOLON] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_SLASH] = {NULL, binary, PRECEDENCE_FACTOR},
    [TOKEN_STAR] = {NULL, binary, PRECEDENCE_FACTOR},

    [TOKEN_BANG] = {unary, NULL, PRECEDENCE_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PRECEDENCE_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PRECEDENCE_EQUALITY},
    [TOKEN_GREATER] = {NULL, binary, PRECEDENCE_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PRECEDENCE_COMPARISON},
    [TOKEN_LESS] = {NULL, binary, PRECEDENCE_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PRECEDENCE_COMPARISON},

    [TOKEN_IDENTIFIER] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_STRING] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_NUMBER] = {number, NULL, PRECEDENCE_NONE},

    [TOKEN_AND] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_CLASS] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FALSE] = {literal, NULL, PRECEDENCE_NONE},
    [TOKEN_FOR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_FUN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_IF] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_NIL] = {literal, NULL, PRECEDENCE_NONE},
    [TOKEN_OR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_SUPER] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_THIS] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_TRUE] = {literal, NULL, PRECEDENCE_NONE},
    [TOKEN_VAR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PRECEDENCE_NONE},

    [TOKEN_ERROR] = {NULL, NULL, PRECEDENCE_NONE},
    [TOKEN_EOF] = {NULL, NULL, PRECEDENCE_NONE},
};

static ParseRule *get_rule(TokenType type) { return &rules[type]; }

bool compile(const char *source_code, Chunk *chunk) {
    init_scanner(source_code);
    compiling_chunk = chunk;

    parser.had_error = false;
    parser.panic_mode = false;

    advance();
    expression();

    consume(TOKEN_EOF, "Expected end of expression.");
    end_compiler();
    return !parser.had_error;
}
