#include "ds/list/container.h"
#include "ds/list/list.h"
#include "networking/discover.h"
#include "networking/types.h"
#include "store/node.h"
#include <arpa/inet.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>

# define PORT 5125
# define POLL_MS 2000
# define TCP_PORT 1226
# define TTL_MS 60 * 60 * 5

uint64_t now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void process_discovered(
    const struct discovery_payload* payload,
    const struct sockaddr_in* addr,
    void *ctx
){
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
struct discovery_ctx {
    char node_id[NODE_ID_BYTES];
    struct device_table *table;
};
void *discovery_thread(void *arg)
{
    struct discovery_ctx *ctx = arg;

    int dstatus = discovery(
        PORT,
        TCP_PORT,
        POLL_MS,
        ctx->node_id,
        process_discovered,
        ctx->table
    );

    if (dstatus != 0) {
        perror("discovery failed");
    }

    return NULL;
}

int main(void) {
	char node_id[NODE_ID_BYTES];
	if (load_or_create_node_id(node_id) != 0) {
		fprintf(stderr, "failed to load/create node_id\n");
		return 1;
	}

	struct device_table table = {
		.count = 0,
		.ttl_ms = TTL_MS,
		.lru = { .next = &table.lru, .prev = &table.lru }
	};

	printf("Discovery listening on UDP %d and broadcasting every %d ms\n", PORT, POLL_MS);
	fflush(stdout);
	
	struct discovery_ctx ctx = {
		.table = &table
	};
	memcpy(ctx.node_id, node_id, NODE_ID_BYTES);

	pthread_t tid;
	pthread_create(&tid, NULL, discovery_thread, &ctx);
	
    while (1) {
        sleep(5);

        struct list_head *pos;
        printf("Current devices:\n");
        for (pos = table.lru.next; pos != &table.lru; pos = pos->next) {
            struct device_node *dev = container_of(pos, struct device_node, lru);
            printf(" - last_seen %llu ms ago\n", (unsigned long long)(now_ms() - dev->last_seen));
        }
    }

    pthread_join(tid, NULL);
	return 0;
}
