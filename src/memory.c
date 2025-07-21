#include "memory.h"

#include <stdlib.h>

#include "object.h"
#include "virtual_machine.h"

void *reallocate(void *pointer, size_t old_size, size_t new_size) {
    if (new_size == 0) {
        free(pointer);
        return NULL;
    }

    void *result = realloc(pointer, new_size);
    if (result == NULL) {
        exit(1);
    }

    return result;
}

static void free_object(Object *object) {
    switch (object->type) {
        case OBJECT_STRING: {
            ObjectString *string = (ObjectString *)object;
            FREE_ARRAY(char, string->characters, string->length + 1);
            FREE(ObjectString, object);
            break;
        }
    }
}

void free_objects() {
    Object *object = vm.objects;
    while (object != NULL) {
        Object *next = object->next;
        free_object(object);
        object = next;
    }
}
