#include "networking/discover.h"
#include "networking/types.h"
#include "store/node.h"
#include <arpa/inet.h>
#include <stdio.h>

# define PORT 5125
# define POLL_MS 2000
# define TCP_PORT 1226

void print_disc( 
	const struct discovery_payload* payload,
	const struct sockaddr_in* addr
){
	printf("Found %s offering port %d, at %s\n", payload->hostname, payload->port, inet_ntoa(addr->sin_addr));
	fflush(stdout);
}

int main(void) {
	char node_id[NODE_ID_BYTES];
	if (load_or_create_node_id(node_id) != 0) {
		fprintf(stderr, "failed to load/create node_id\n");
		return 1;
	}

	printf("Discovery listening on UDP %d and broadcasting every %d ms\n", PORT, POLL_MS);
	fflush(stdout);
	
	discovery(PORT, TCP_PORT, POLL_MS, node_id, print_disc);
	return 0;
}
