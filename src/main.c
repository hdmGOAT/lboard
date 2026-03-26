#include "networking/discover.h"
#include "networking/types.h"
#include "store/node.h"
#include <arpa/inet.h>
#include <stdio.h>

# define PORT 5125
# define POLL_MS 2000
# define TCP_PORT 1226
# define TTL_MS 60 * 60
void process_discovered( 
	const struct discovery_payload* payload,
	const struct sockaddr_in* addr
){
	
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
	
	discovery(PORT, TCP_PORT, POLL_MS, node_id, process_discovered);
	return 0;
}
