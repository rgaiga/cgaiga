#ifndef cgaiga_memory_h
#define cgaiga_memory_h

#include "common.h"

void *reallocate(void *pointer, size_t old_size, size_t new_size);

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, old_size, new_size) \
    (type *)reallocate(pointer, sizeof(type) * (old_size), sizeof(type) * new_size)

#define FREE_ARRAY(type, pointer, size) reallocate(pointer, sizeof(type) * size, 0)

#endif
