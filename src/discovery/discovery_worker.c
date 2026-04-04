#include "discovery_worker.h"

#include "devices/device_table.h"
#include "networking/discover.h"

#include <pthread.h>
#include <stdatomic.h>
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
    atomic_int stop;
};

struct discovery_worker_state {
    pthread_t thread;
    struct discovery_ctx *ctx;
    int running;
};

static struct discovery_worker_state g_worker = {0};

static int discovery_should_stop(void *ctx) {
    struct discovery_ctx *discovery_ctx = ctx;
    return atomic_load(&discovery_ctx->stop) != 0;
}

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
        ctx,
        discovery_should_stop,
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
    struct discovery_ctx *ctx = malloc(sizeof(*ctx));

    if (g_worker.running) {
        fprintf(stderr, "discovery worker already running\n");
        return -1;
    }

    if (ctx == NULL) {
        perror("malloc failed");
        return -1;
    }

    ctx->table = table;
    memcpy(ctx->node_id, node_id, NODE_ID_BYTES);
    atomic_store(&ctx->stop, 0);

    if (pthread_create(&g_worker.thread, NULL, discovery_thread, ctx) != 0) {
        perror("pthread_create");
        free(ctx);
        return -1;
    }

    g_worker.ctx = ctx;
    g_worker.running = 1;

    return 0;
}

int stop_discovery_worker(void) {
    if (!g_worker.running) {
        return 0;
    }

    atomic_store(&g_worker.ctx->stop, 1);

    if (pthread_join(g_worker.thread, NULL) != 0) {
        perror("pthread_join");
        return -1;
    }

    g_worker.ctx = NULL;
    g_worker.running = 0;
    return 0;
}
