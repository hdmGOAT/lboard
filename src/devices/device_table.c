#include "device_table.h"

#include "ds/hashmap/hashmap.h"
#include "ds/list/list.h"
#include "networking/types.h"
#include "time/clock.h"
#include "ds/list/container.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t rendered_lines = 0;

void device_table_init(struct device_table *table, uint64_t ttl_ms) {
    int status;

    table->count = 0;
    table->ttl_ms = ttl_ms;
    table->lru.next = &table->lru;
    table->lru.prev = &table->lru;

    status = pthread_mutex_init(&table->mutex, NULL);
    if (status != 0) {
        fprintf(stderr, "pthread_mutex_init failed\n");
        abort();
    }
}

struct device_node *device_table_add(struct device_table *table,
                 const struct discovery_payload *payload) {
    struct device_node *dev;

    dev = malloc(sizeof(*dev));
    if (dev == NULL) {
        perror("malloc failed");
        return NULL;
    }

    memcpy(&dev->payload, payload, sizeof(*payload));
    dev->last_seen = now_ms();
    INIT_LIST_HEAD(&dev->lru);

    pthread_mutex_lock(&table->mutex);
    list_add(&dev->lru, &table->lru);
    table->count++;
    pthread_mutex_unlock(&table->mutex);

    return dev;
}

void device_table_touch(struct device_table *table,
            struct device_node *device,
            const struct discovery_payload *payload) {
    if (table == NULL || device == NULL || payload == NULL) {
        return;
    }

    pthread_mutex_lock(&table->mutex);
    memcpy(&device->payload, payload, sizeof(*payload));
    device->last_seen = now_ms();
    list_del(&device->lru);
    list_add(&device->lru, &table->lru);
    pthread_mutex_unlock(&table->mutex);
}

void device_table_on_discovered(
    const struct discovery_payload *payload,
    const struct sockaddr_in *addr,
    void *ctx
) {
    (void)addr;

    struct device_table *table = ctx;
    (void)device_table_add(table, payload);
}

void device_table_print(struct device_table *table) {
    const struct list_head *pos;
    size_t lines = 0;
    size_t old_lines = rendered_lines;

    pthread_mutex_lock(&table->mutex);

    if (old_lines > 0) {
        printf("\033[%zuA", old_lines);
    }

    printf("\r\033[2KCurrent devices:\n");
    lines++;

    for (pos = table->lru.next; pos != &table->lru; pos = pos->next) {
        const struct device_node *dev = container_of(pos, struct device_node, lru);
        printf("\r\033[2K%s\n", dev->payload.hostname);
        printf("\r\033[2K - last_seen %llu ms ago\n", (unsigned long long)(now_ms() - dev->last_seen));
        lines += 2;
    }

    if (old_lines > lines) {
        size_t extra = old_lines - lines;
        for (size_t i = 0; i < extra; i++) {
            printf("\r\033[2K\n");
        }
        printf("\033[%zuA", extra);
    }

    rendered_lines = lines;
    fflush(stdout);
    pthread_mutex_unlock(&table->mutex);
}

void device_table_expire_devices(struct device_table *table,
                                 struct hashmap *device_map) {
    struct list_head *pos;
    uint64_t now;

    if (table == NULL || device_map == NULL) {
        return;
    }

    pthread_mutex_lock(&table->mutex);
    now = now_ms();
    pos = table->lru.next;

    while (pos != &table->lru) {
        struct list_head *next = pos->next;
        struct device_node *dev = container_of(pos, struct device_node, lru);

        if (now - dev->last_seen < table->ttl_ms) {
            break;
        }

        list_del(&dev->lru);
        (void)hashmap_remove(device_map, dev->payload.node_id);
        free(dev);

        if (table->count > 0) {
            table->count--;
        }

        pos = next;
    }
    pthread_mutex_unlock(&table->mutex);
}
