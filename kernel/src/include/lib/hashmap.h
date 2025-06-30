#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    void *value;
    size_t size;
} hashmap_object;

typedef struct bucket bucket;
typedef struct bucket {
    uint32_t hash;
    char *key;
    hashmap_object object;
    bucket *next;
} bucket;

typedef struct {
    bucket **buckets;
    size_t bucket_count, node_count;
} hashmap;

hashmap hashmap_new();
void hashmap_set(hashmap *self, const char *key, void *value, size_t size);
hashmap_object *hashmap_get(hashmap *self, const char *key);
hashmap_object *hashmap_at(hashmap *self, size_t i);
bool hashmap_remove(hashmap *self, const char *key);
void hashmap_free(hashmap *self);
