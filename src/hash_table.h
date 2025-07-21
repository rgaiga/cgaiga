#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "common.h"
#include "value.h"

typedef struct {
    ObjectString* key;
    Value value;
} Entry;

typedef struct {
    int size;
    int capacity;
    Entry* entries;
} HashTable;

void init_hash_table(HashTable* hash_table);
void free_hash_table(HashTable* hash_table);

bool hash_table_set(HashTable* hash_table, ObjectString* key, Value value);
// TODO: Isn't it better to return a pointer to the value or NULL instead?
bool hash_table_get(HashTable* hash_table, ObjectString* key, Value* value);
bool hash_table_delete(HashTable* hash_table, ObjectString* key);
void hash_table_copy(HashTable* source, HashTable* destination);

ObjectString* hash_table_find_string(HashTable* hash_table, const char* characters, int length,
                                     uint32_t hash);

#endif
