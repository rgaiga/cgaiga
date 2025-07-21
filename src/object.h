#ifndef OBJECT_H
#define OBJECT_H

#include "common.h"
#include "value.h"

#define OBJECT_TYPE(value) (AS_OBJECT(value)->type)

#define IS_STRING(value) is_object_type(value, OBJECT_STRING)

#define AS_STRING(value) ((ObjectString*)AS_OBJECT(value))
#define AS_CSTRING(value) ((ObjectString*)AS_OBJECT(value)->characters)

typedef enum { OBJECT_STRING } ObjectType;

struct Object {
    ObjectType type;
    struct Object* next;
};

struct ObjectString {
    Object object;
    int length;
    char* characters;
    uint32_t hash;
};

ObjectString* take_string(char* characters, int length);
ObjectString* copy_string(const char* characters, int length);

static inline bool is_object_type(Value value, ObjectType type) {
    return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif
