#include "ds/hashmap/hashmap.h"

#include <errno.h>
#include <stdlib.h>

enum {
	HASHMAP_DEFAULT_CAPACITY_BITS = 4,
};

static size_t hashmap_bucket_index(const struct hashmap *map, const void *key) {
	size_t hash = map->hash_fn(key, map->ctx);
	return hash & (map->capacity - 1);
}

int hashmap_init(struct hashmap *map, hashmap_hash_fn hash_fn,
		hashmap_equal_fn equal_fn, void *ctx) {
	size_t capacity;

	if (map == NULL || hash_fn == NULL || equal_fn == NULL) {
		errno = EINVAL;
		return -1;
	}

	capacity = (size_t)1u << HASHMAP_DEFAULT_CAPACITY_BITS;

	map->buckets = calloc(capacity, sizeof(*map->buckets));
	if (map->buckets == NULL) {
		return -1;
	}

	map->hash_fn = hash_fn;
	map->equal_fn = equal_fn;
	map->ctx = ctx;
	map->capacity = capacity;
	map->capacity_bits = HASHMAP_DEFAULT_CAPACITY_BITS;
	map->size = 0;

	return 0;
}

struct hashmap *hashmap_new(hashmap_hash_fn hash_fn,
			hashmap_equal_fn equal_fn,
			void *ctx) {
	struct hashmap *map;

	map = malloc(sizeof(*map));
	if (map == NULL) {
		return NULL;
	}

	if (hashmap_init(map, hash_fn, equal_fn, ctx) != 0) {
		free(map);
		return NULL;
	}

	return map;
}

void hashmap_clear(struct hashmap *map) {
	size_t index;

	if (map == NULL || map->buckets == NULL) {
		return;
	}

	for (index = 0; index < map->capacity; ++index) {
		struct hashmap_entry *entry = map->buckets[index];

		while (entry != NULL) {
			struct hashmap_entry *next = entry->next;
			free(entry);
			entry = next;
		}

		map->buckets[index] = NULL;
	}

	map->size = 0;
}

void hashmap_destroy(struct hashmap *map) {
	if (map == NULL) {
		return;
	}

	hashmap_clear(map);
	free(map->buckets);

	map->buckets = NULL;
	map->capacity = 0;
	map->capacity_bits = 0;
	map->size = 0;
	map->hash_fn = NULL;
	map->equal_fn = NULL;
	map->ctx = NULL;
}

void hashmap_free(struct hashmap *map) {
	if (map == NULL) {
		return;
	}

	hashmap_destroy(map);
	free(map);
}

int hashmap_insert(struct hashmap *map, const void *key, void *value) {
	size_t index;
	struct hashmap_entry *entry;

	if (map == NULL || map->buckets == NULL || key == NULL) {
		errno = EINVAL;
		return -1;
	}

	index = hashmap_bucket_index(map, key);
	entry = map->buckets[index];
	while (entry != NULL) {
		if (map->equal_fn(entry->key, key, map->ctx) != 0) {
			entry->value = value;
			return 0;
		}
		entry = entry->next;
	}

	entry = malloc(sizeof(*entry));
	if (entry == NULL) {
		return -1;
	}

	entry->key = key;
	entry->value = value;
	entry->next = map->buckets[index];
	map->buckets[index] = entry;
	map->size++;

	return 0;
}

void *hashmap_get(const struct hashmap *map, const void *key) {
	size_t index;
	struct hashmap_entry *entry;

	if (map == NULL || map->buckets == NULL || key == NULL) {
		return NULL;
	}

	index = hashmap_bucket_index(map, key);
	entry = map->buckets[index];
	while (entry != NULL) {
		if (map->equal_fn(entry->key, key, map->ctx) != 0) {
			return entry->value;
		}
		entry = entry->next;
	}

	return NULL;
}

void *hashmap_remove(struct hashmap *map, const void *key) {
	size_t index;
	struct hashmap_entry *entry;
	struct hashmap_entry *prev;

	if (map == NULL || map->buckets == NULL || key == NULL) {
		return NULL;
	}

	index = hashmap_bucket_index(map, key);
	entry = map->buckets[index];
	prev = NULL;

	while (entry != NULL) {
		if (map->equal_fn(entry->key, key, map->ctx) != 0) {
			void *value = entry->value;

			if (prev == NULL) {
				map->buckets[index] = entry->next;
			} else {
				prev->next = entry->next;
			}

			free(entry);
			map->size--;
			return value;
		}

		prev = entry;
		entry = entry->next;
	}

	return NULL;
}
