#include "value.h"

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"

void init_value_array(ValueArray *array) {
    array->size = 0;
    array->capacity = 0;
    array->values = NULL;
}

void free_value_array(ValueArray *array) {
    FREE_ARRAY(Value, array->values, array->size);
    init_value_array(array);
}

void write_value_array(ValueArray *array, Value value) {
    if (array->capacity < array->size + 1) {
        int old_capacity = array->capacity;

        array->capacity = GROW_CAPACITY(old_capacity);
        array->values = GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
    }

    array->values[array->size] = value;
    array->size++;
}

void print_object(Value value) {
    // printf("print_object: type: %d\n", OBJECT_TYPE(value));

    switch (OBJECT_TYPE(value)) {
        case OBJECT_STRING:
            printf("%s", AS_STRING(value)->characters);
            break;
    }
}

void print_value(Value value) {
    switch (value.type) {
        case VALUE_BOOLEAN:
            printf(AS_BOOLEAN(value) ? "true" : "false");
            break;
        case VALUE_NIL:
            printf("nil");
            break;
        case VALUE_NUMBER:
            printf("%g", AS_NUMBER(value));
            break;
        case VALUE_OBJECT:
            print_object(value);
            break;
    }
}

bool values_equal(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case VALUE_BOOLEAN:
            return AS_BOOLEAN(a) == AS_BOOLEAN(b);
        case VALUE_NIL:
            return true;
        case VALUE_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);
        case VALUE_OBJECT: {
            ObjectString *string_a = AS_STRING(a);
            ObjectString *string_b = AS_STRING(b);
            return string_a->length == string_b->length &&
                   memcmp(string_a->characters, string_b->characters, string_a->length) == 0;
        }
        default:
            return false;  // Unreachable.
    }
}
