#include "hash_table.h"

#include <string.h>

#include "memory.h"
#include "object.h"

#define HASH_TABLE_MAX_LOAD 0.75

void init_hash_table(HashTable* hash_table) {
    hash_table->size = 0;
    hash_table->capacity = 0;
    hash_table->entries = NULL;
}

void free_hash_table(HashTable* hash_table) {
    FREE_ARRAY(Entry, hash_table->entries, hash_table->capacity);
    init_hash_table(hash_table);
}

static Entry* find_entry(Entry* entries, int capacity, ObjectString* key) {
    uint32_t index = key->hash % capacity;
    Entry* tombstone = NULL;

    for (;;) {
        Entry* entry = &entries[index];
        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // Entry is indeed empty.
                return tombstone != NULL ? tombstone : entry;
            } else {
                // We found a tombstone.
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            // We found the entry.
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

static void adjust_capacity(HashTable* hash_table, int capacity) {
    // Allocate memory for the new array.
    Entry* entries = ALLOCATE(Entry, capacity);

    // Initialize all entries to be empty.
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VALUE;
    }

    // Reset the hash table's size so that tombstones are discarded.
    hash_table->size = 0;

    // Reinsert all entries into the new empty array.
    for (int i = 0; i < hash_table->capacity; i++) {
        Entry* entry = &hash_table->entries[i];

        // Skip empty entries.
        if (entry->key == NULL) continue;

        Entry* destination = find_entry(entries, capacity, entry->key);
        destination->key = entry->key;
        destination->value = entry->value;

        hash_table->size++;
    }

    // Free the old array.
    FREE_ARRAY(Entry, hash_table->entries, hash_table->capacity);

    // Updates the hash table to point to the new array.
    hash_table->entries = entries;
    hash_table->capacity = capacity;
}

bool hash_table_set(HashTable* hash_table, ObjectString* key, Value value) {
    if (hash_table->size + 1 > hash_table->capacity * HASH_TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(hash_table->capacity);
        adjust_capacity(hash_table, capacity);
    }

    Entry* entry = find_entry(hash_table->entries, hash_table->capacity, key);
    bool is_new_key = entry->key == NULL;

    // Only increment the hash table's size if the entry is an entirely new
    // bucket (not a tombstone).
    if (is_new_key && IS_NIL(entry->value)) hash_table->size++;

    entry->key = key;
    entry->value = value;
    return is_new_key;
}

bool hash_table_get(HashTable* hash_table, ObjectString* key, Value* value) {
    if (hash_table->size == 0) return false;

    Entry* entry = find_entry(hash_table->entries, hash_table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

bool hash_table_delete(HashTable* hash_table, ObjectString* key) {
    if (hash_table->size == 0) return false;

    Entry* entry = find_entry(hash_table->entries, hash_table->capacity, key);
    if (entry->key == NULL) return false;

    entry->key = NULL;
    entry->value = BOOLEAN_VALUE(true);
    return true;
}

void hash_table_copy(HashTable* source, HashTable* destination) {
    for (int i = 0; i < source->capacity; i++) {
        Entry* entry = &source->entries[i];

        // Skip empty entries.
        if (entry->key == NULL) continue;

        hash_table_set(destination, entry->key, entry->value);
    }
}

ObjectString* hash_table_find_string(HashTable* hash_table, const char* characters, int length,
                                     uint32_t hash) {
    if (hash_table->size == 0) return NULL;

    uint32_t index = hash % hash_table->capacity;
    for (;;) {
        Entry* entry = &hash_table->entries[index];
        if (entry->key == NULL) {
            // Stop if we find an empty non-tombstone entry.
            if (IS_NIL(entry->value)) return NULL;
        } else if (entry->key->length == length && entry->key->hash == hash &&
                   memcmp(entry->key->characters, characters, length) == 0) {
            // We found the string.
            return entry->key;
        }

        index = (index + 1) % hash_table->capacity;
    }
}
