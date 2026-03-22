#include "networking/discover.h"
#include "store/node.h"
#include <stdio.h>

# define PORT 5125
# define POLL_MS 2000
# define TCP_PORT 1226

int main(void) {
	char node_id[NODE_ID_BYTES];
	if (load_or_create_node_id(node_id) != 0) {
		fprintf(stderr, "failed to load/create node_id\n");
		return 1;
	}

	discovery(PORT,TCP_PORT,POLL_MS, uid());
	return 0;
}
