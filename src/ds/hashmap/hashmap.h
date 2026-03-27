#ifndef LBOARD_DS_HASHMAP_HASHMAP_H
#define LBOARD_DS_HASHMAP_HASHMAP_H

#include <stddef.h>

typedef size_t (*hashmap_hash_fn)(const void *key, void *ctx);
typedef int (*hashmap_equal_fn)(const void *left_key, const void *right_key,
				void *ctx);

struct hashmap_entry {
	const void *key;
	void *value;
	struct hashmap_entry *next;
};

struct hashmap {
	hashmap_hash_fn hash_fn;
	hashmap_equal_fn equal_fn;
	void *ctx;

	struct hashmap_entry **buckets;
	size_t capacity;
	size_t capacity_bits;
	size_t size;
};

int hashmap_init(struct hashmap *map, hashmap_hash_fn hash_fn,
		hashmap_equal_fn equal_fn, void *ctx);
struct hashmap *hashmap_new(hashmap_hash_fn hash_fn,
			hashmap_equal_fn equal_fn,
			void *ctx);
void hashmap_clear(struct hashmap *map);
void hashmap_destroy(struct hashmap *map);
void hashmap_free(struct hashmap *map);

int hashmap_insert(struct hashmap *map, const void *key, void *value);
void *hashmap_get(const struct hashmap *map, const void *key);
void *hashmap_remove(struct hashmap *map, const void *key);

#define hashmap__init hashmap_init
#define hashmap__new hashmap_new
#define hashmap__clear hashmap_clear
#define hashmap__free hashmap_free

#endif
