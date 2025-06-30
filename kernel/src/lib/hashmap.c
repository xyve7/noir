#include <lib/hashmap.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <stdint.h>

uint32_t djb33_hash(const char *s, size_t len) {
    uint32_t h = 5381;
    while (len--) {
        /* h = 33 * h ^ s[i]; */
        h += (h << 5);
        h ^= *s++;
    }
    return h;
}
hashmap hashmap_new() {
    hashmap self;
    self.bucket_count = 10;
    self.node_count = 0;
    self.buckets = kmalloc(self.bucket_count * sizeof(bucket));
    memset(self.buckets, 0, self.bucket_count * sizeof(bucket));

    return self;
}
void hashmap_set(hashmap *self, const char *key, void *value, size_t size) {
    uint32_t string_hash = djb33_hash(key, strlen(key));
    size_t index = string_hash % self->bucket_count;

    bucket *new_bucket = kmalloc(sizeof(bucket));
    new_bucket->hash = string_hash;
    new_bucket->key = strdup(key);
    new_bucket->object.value = value;
    new_bucket->object.size = size;

    new_bucket->next = self->buckets[index];
    self->buckets[index] = new_bucket;

    self->node_count++;
}
hashmap_object *hashmap_get(hashmap *self, const char *key) {
    uint32_t string_hash = djb33_hash(key, strlen(key));
    size_t index = string_hash % self->bucket_count;

    bucket *b = self->buckets[index];
    while (b) {
        if (b->hash == djb33_hash(key, strlen(key))) {
            return &b->object;
        }
        b = b->next;
    }

    return nullptr;
}
hashmap_object *hashmap_at(hashmap *self, size_t i) {
    size_t count = 0;
    for (size_t j = 0; j < self->bucket_count; j++) {
        bucket *b = self->buckets[j];
        while (b) {
            if (count == i) {
                return &b->object;
            }
            count++;
            b = b->next;
        }
    }

    return nullptr;
}
void free_bucket_chain(bucket *b) {
    if (b->next == nullptr) {
        kfree(b->key);
        kfree(b->object.value);
        kfree(b);
        return;
    }
    free_bucket_chain(b->next);
}
void hashmap_kfree(hashmap *self) {
    for (size_t i = 0; i < self->bucket_count; i++) {
        free_bucket_chain(self->buckets[i]);
    }
}
