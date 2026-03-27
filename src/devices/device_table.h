#ifndef DEVICE_TABLE_H
#define DEVICE_TABLE_H

#include "networking/types.h"

#include <netinet/in.h>
#include <stdint.h>

void device_table_init(struct device_table *table, uint64_t ttl_ms);
struct device_node *device_table_add(struct device_table *table,
			     const struct discovery_payload *payload);
void device_table_touch(struct device_table *table,
            struct device_node *device,
			const struct discovery_payload *payload);
void device_table_on_discovered(
    const struct discovery_payload *payload,
    const struct sockaddr_in *addr,
    void *ctx
);
void device_table_print(const struct device_table *table);

#endif
