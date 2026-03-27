#include "discovery_worker.h"

#include "devices/device_table.h"
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

struct discovery_ctx {
    char node_id[NODE_ID_BYTES];
    struct device_table *table;
};

static void discovery_on_device(
    const struct discovery_payload *payload,
    const struct sockaddr_in *addr,
    void *ctx
) {
    struct discovery_ctx *discovery_ctx = ctx;

    (void)addr;

    (void)device_table_upsert(discovery_ctx->table, payload);
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

    if (pthread_create(&thread, NULL, discovery_thread, ctx) != 0) {
        perror("pthread_create");
        free(ctx);
        return -1;
    }

    pthread_detach(thread);
    return 0;
}
