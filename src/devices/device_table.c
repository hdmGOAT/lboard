#include "device_table.h"

#include "time/clock.h"
#include "ds/list/container.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void device_table_init(struct device_table *table, uint64_t ttl_ms) {
    table->count = 0;
    table->ttl_ms = ttl_ms;
    table->lru.next = &table->lru;
    table->lru.prev = &table->lru;
}

void device_table_on_discovered(
    const struct discovery_payload *payload,
    const struct sockaddr_in *addr,
    void *ctx
) {
    (void)addr;

    struct device_table *table = ctx;

    struct device_node *dev = malloc(sizeof(*dev));
    if (dev == NULL) {
        perror("malloc failed");
        return;
    }

    memcpy(&dev->payload, payload, sizeof(*payload));
    dev->last_seen = now_ms();
    INIT_LIST_HEAD(&dev->lru);

    list_add(&dev->lru, &table->lru);
}

void device_table_print(const struct device_table *table) {
    const struct list_head *pos;

    printf("Current devices:\n");
    for (pos = table->lru.next; pos != &table->lru; pos = pos->next) {
        const struct device_node *dev = container_of(pos, struct device_node, lru);
        printf(" - last_seen %llu ms ago\n", (unsigned long long)(now_ms() - dev->last_seen));
    }
}
