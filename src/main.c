#include "ds/list/list.h"
#include "networking/discover.h"
#include "networking/types.h"
#include "store/node.h"
#include <arpa/inet.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define PORT 5125
# define POLL_MS 2000
# define TCP_PORT 1226
# define TTL_MS 60 * 60
#include <sys/time.h>
#include <stdint.h>

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
	
	int dstatus = discovery(PORT, TCP_PORT, POLL_MS, node_id, process_discovered, &table);
	return 0;
}
