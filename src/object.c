#include "object.h"

#include <stdio.h>
#include <string.h>

#include "hash_table.h"
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

static ObjectString* allocate_string(char* characters, int length, uint32_t hash) {
    ObjectString* object_string = ALLOCATE_OBJECT(ObjectString, OBJECT_STRING);
    object_string->characters = characters;
    object_string->length = length;
    object_string->hash = hash;

    // Intern the string (using the hash table as a hash set...).
    hash_table_set(&vm.strings, object_string, NIL_VALUE);

    return object_string;
}

// FNV-1a
static uint32_t hash_string(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjectString* take_string(char* characters, int length) {
    uint32_t hash = hash_string(characters, length);

    ObjectString* interned_string = hash_table_find_string(&vm.strings, characters, length, hash);
    if (interned_string != NULL) {
        FREE_ARRAY(char, characters, length);
        return interned_string;
    }

    return allocate_string(characters, length, hash);
}

ObjectString* copy_string(const char* characters, int length) {
    uint32_t hash = hash_string(characters, length);

    ObjectString* interned_string = hash_table_find_string(&vm.strings, characters, length, hash);
    if (interned_string != NULL) return interned_string;

    char* string_copy = ALLOCATE(char, length + 1);  // Length + 1 for null terminator
    memcpy(string_copy, characters, length);
    string_copy[length] = '\0';

    return allocate_string(string_copy, length, hash);
}
