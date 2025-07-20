#ifndef VALUE_H
#define VALUE_H

#include "common.h"

typedef enum {
    VALUE_BOOLEAN,
    VALUE_NUMBER,
    VALUE_NIL,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
    };
} Value;

#define IS_BOOLEAN(value) ((value).type == VALUE_BOOLEAN)
#define IS_NUMBER(value) ((value).type == VALUE_NUMBER)
#define IS_NIL(value) ((value).type == VALUE_NIL)

#define AS_BOOLEAN(value) ((value).boolean)
#define AS_NUMBER(value) ((value).number)

#define BOOLEAN_VALUE(value) ((Value){VALUE_BOOLEAN, {.boolean = value}})
#define NUMBER_VALUE(value) ((Value){VALUE_NUMBER, {.number = value}})
#define NIL_VALUE ((Value){VALUE_NIL, {.number = 0}})

typedef struct {
    int size;
    int capacity;
    Value *values;
} ValueArray;

void init_value_array(ValueArray *array);
void free_value_array(ValueArray *array);
void write_value_array(ValueArray *array, Value value);

void print_value(Value value);
bool values_equal(Value a, Value b);

#endif
