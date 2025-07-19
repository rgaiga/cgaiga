#include "scanner.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    ESCAPE_SEQUENCE_CARRIAGE_RETURN = '\r',
    ESCAPE_SEQUENCE_NEWLINE = '\n',
    ESCAPE_SEQUENCE_NULL = '\0',
    ESCAPE_SEQUENCE_TAB = '\t',
} EscapeSequence;

typedef struct {
    const char *start;    // Pointer to Lexeme's start character.
    const char *current;  // Pointer to Lexeme's current character.
    int current_line;
} Scanner;

Scanner scanner;

void init_scanner(const char *source_code) {
    scanner.start = source_code;
    scanner.current = source_code;
    scanner.current_line = 1;
}

static bool is_digit(char character) { return character >= '0' && character <= '9'; }
static bool is_alpha(char character) {
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
           character == '_';
}

static bool is_at_end() { return *scanner.current == ESCAPE_SEQUENCE_NULL; }

static Token make_token(TokenType type) {
    Token token;
    token.type = type;
    token.line = scanner.current_line;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);  // Pointer offset casted to integer.

    return token;
}

static Token make_error_token(const char *message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.line = scanner.current_line;
    token.start = message;
    token.length = (int)strlen(message);

    return token;
}

static char advance() { return *scanner.current++; }

static char peek() { return *scanner.current; }

static char peek_next() {
    if (is_at_end()) return ESCAPE_SEQUENCE_NULL;
    return *(scanner.current + 1);
}

static void skip_whitespace() {
    for (;;) {
        char character = peek();

        switch (character) {
            case ' ':
            case ESCAPE_SEQUENCE_CARRIAGE_RETURN:
            case ESCAPE_SEQUENCE_TAB:
                advance();
                break;
            case ESCAPE_SEQUENCE_NEWLINE:
                scanner.current_line++;
                advance();
                break;
            case '/': {
                // Single-line comment.
                if (peek_next() == '/') {
                    // A single-line comment goes until the end of the line.
                    while (!is_at_end()) {
                        if (peek() == ESCAPE_SEQUENCE_NEWLINE) break;
                        advance();
                    }
                }
                break;
            }
            default:
                return;
        }
    }
}

static TokenType check_keyword(int start, int length, const char *rest, TokenType type) {
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifier_type() {
    switch (scanner.start[0]) {
        case 'a':
            return check_keyword(1, 2, "nd", TOKEN_AND);
        case 'c':
            return check_keyword(1, 4, "lass", TOKEN_CLASS);
        case 'e':
            return check_keyword(1, 3, "lse", TOKEN_ELSE);
        case 'i':
            return check_keyword(1, 1, "f", TOKEN_IF);
        case 'n':
            return check_keyword(1, 2, "il", TOKEN_NIL);
        case 'o':
            return check_keyword(1, 1, "r", TOKEN_OR);
        case 'p':
            return check_keyword(1, 4, "rint", TOKEN_PRINT);
        case 'r':
            return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 's':
            return check_keyword(1, 4, "uper", TOKEN_SUPER);
        case 'v':
            return check_keyword(1, 2, "ar", TOKEN_VAR);
        case 'w':
            return check_keyword(1, 4, "hile", TOKEN_WHILE);
        case 'f':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a':
                        return check_keyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o':
                        return check_keyword(2, 1, "r", TOKEN_FOR);
                    case 'u':
                        return check_keyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h':
                        return check_keyword(2, 2, "is", TOKEN_THIS);
                    case 'r':
                        return check_keyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
    }

    return TOKEN_IDENTIFIER;
}

// Literal: identifier
static Token identifier() {
    for (;;) {
        char character = peek();
        if (!is_alpha(character) && !is_digit(character)) break;
        advance();
    }

    return make_token(identifier_type());
}

// Literal: number
static Token number() {
    for (;;) {
        char character = peek();
        if (!is_digit(character)) break;
        advance();
    }

    // Look for a fractional part.
    if (peek() == '.' && is_digit(peek_next())) {
        advance();  // consume '.'

        for (;;) {
            char character = peek();
            if (!is_digit(character)) break;
            advance();
        }
    }

    return make_token(TOKEN_NUMBER);
}

// Literal: string
static Token string() {
    while (!is_at_end()) {
        if (peek() == '"') break;  // Reached the ending quote.
        if (peek() == ESCAPE_SEQUENCE_NEWLINE) scanner.current_line++;
        advance();
    }

    if (is_at_end()) return make_error_token("Unterminated string.");

    advance();  // consume '"'

    return make_token(TOKEN_STRING);
}

static bool match(char expected_character) {
    if (is_at_end()) return false;
    if (*scanner.current != expected_character) return false;

    scanner.current++;
    return true;
}

Token scan_token() {
    skip_whitespace();
    scanner.start = scanner.current;

    if (is_at_end()) return make_token(TOKEN_EOF);

    char character = advance();
    if (is_alpha(character)) return identifier();
    if (is_digit(character)) return number();

    switch (character) {
        // Single-character tokens.
        case '(':
            return make_token(TOKEN_LEFT_PARENTHESIS);
        case ')':
            return make_token(TOKEN_RIGHT_PARENTHESIS);
        case '{':
            return make_token(TOKEN_LEFT_BRACE);
        case '}':
            return make_token(TOKEN_RIGHT_BRACE);
        case ';':
            return make_token(TOKEN_SEMICOLON);
        case ',':
            return make_token(TOKEN_COMMA);
        case '.':
            return make_token(TOKEN_DOT);
        case '-':
            return make_token(TOKEN_MINUS);
        case '+':
            return make_token(TOKEN_PLUS);
        case '*':
            return make_token(TOKEN_STAR);
        case '/':
            return make_token(TOKEN_SLASH);

        // One or two character tokens.
        case '!':
            return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        // String literal.
        case '"':
            return string();
    }

    return make_error_token("Unexpected character.");
}
