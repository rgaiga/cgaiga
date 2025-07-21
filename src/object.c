#include "object.h"

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "value.h"
#include "virtual_machine.h"

#define ALLOCATE_OBJECT(type, object_type) (type*)allocate_object(sizeof(type), object_type)

static Object* allocate_object(size_t size, ObjectType type) {
    Object* object = (Object*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm.objects;
    vm.objects = object;

    return object;
}

static ObjectString* allocate_string(char* characters, int length) {
    ObjectString* object_string = ALLOCATE_OBJECT(ObjectString, OBJECT_STRING);
    object_string->length = length;
    object_string->characters = characters;

    return object_string;
}

ObjectString* take_string(char* characters, int length) {
    return allocate_string(characters, length);
}

ObjectString* copy_string(const char* characters, int length) {
    char* string_copy = ALLOCATE(char, length + 1);  // Length + 1 for null terminator
    memcpy(string_copy, characters, length);
    string_copy[length] = '\0';

    return allocate_string(string_copy, length);
}
