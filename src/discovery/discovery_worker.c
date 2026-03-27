#include "discovery_worker.h"

#include "devices/device_table.h"
#include "ds/hashmap/hashmap.h"
#include "networking/discover.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    DISCOVERY_PORT = 5125,
    DISCOVERY_POLL_MS = 2000,
    DISCOVERY_TCP_PORT = 1226,
};

static size_t node_id_hash(const void *key, void *ctx) {
    const unsigned char *bytes = key;
    size_t hash = 1469598103934665603ull;
    size_t index;
    (void)ctx;

    for (index = 0; index < NODE_ID_BYTES; ++index) {
        hash ^= (size_t)bytes[index];
        hash *= 1099511628211ull;
    }

    return hash;
}

static int node_id_equal(const void *left_key, const void *right_key, void *ctx) {
    (void)ctx;
    return memcmp(left_key, right_key, NODE_ID_BYTES) == 0;
}

struct discovery_ctx {
    char node_id[NODE_ID_BYTES];
    struct device_table *table;
	struct hashmap *device_map;
};

static void discovery_on_device(
    const struct discovery_payload *payload,
    const struct sockaddr_in *addr,
    void *ctx
) {
    struct discovery_ctx *discovery_ctx = ctx;
    struct device_node *device;

    (void)addr;

    device_table_expire_devices(discovery_ctx->table, discovery_ctx->device_map);

    device = hashmap_get(discovery_ctx->device_map, payload->node_id);
    if (device != NULL) {
        device_table_touch(discovery_ctx->table, device, payload);
        return;
    }

    device = device_table_add(discovery_ctx->table, payload);
    if (device == NULL) {
        return;
    }

    if (hashmap_insert(discovery_ctx->device_map, device->payload.node_id, device) != 0) {
        perror("hashmap_insert failed");
    }
}

static void *discovery_thread(void *arg) {
    struct discovery_ctx *ctx = arg;

    int status = discovery(
        DISCOVERY_PORT,
        DISCOVERY_TCP_PORT,
        DISCOVERY_POLL_MS,
        ctx->node_id,
        discovery_on_device,
        ctx
    );

    if (status != 0) {
        perror("discovery failed");
    }

	hashmap_free(ctx->device_map);
    free(ctx);
    return NULL;
}

int start_discovery_worker(
    const char node_id[NODE_ID_BYTES],
    struct device_table *table
) {
    pthread_t thread;
    struct discovery_ctx *ctx = malloc(sizeof(*ctx));
    if (ctx == NULL) {
        perror("malloc failed");
        return -1;
    }

    ctx->table = table;
    memcpy(ctx->node_id, node_id, NODE_ID_BYTES);
	ctx->device_map = hashmap_new(node_id_hash, node_id_equal, NULL);
	if (ctx->device_map == NULL) {
		perror("hashmap_new failed");
		free(ctx);
		return -1;
	}

    if (pthread_create(&thread, NULL, discovery_thread, ctx) != 0) {
        perror("pthread_create");
		hashmap_free(ctx->device_map);
        free(ctx);
        return -1;
    }

    pthread_detach(thread);
    return 0;
}
