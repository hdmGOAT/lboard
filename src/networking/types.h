#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>
#include <netinet/in.h>

#include "../ds/list/linkedist.h"

enum { DISCOVERY_HOSTNAME_SIZE = 64 };
enum { NODE_ID_SIZE = 32 };


struct discovery_payload {
    int32_t port;
	char node_id[NODE_ID_SIZE];
    char hostname[DISCOVERY_HOSTNAME_SIZE];
};


struct device_node {
	struct discovery_payload payload;
	struct list_head lru;
	uint64_t last_seen;
};
enum { DISCOVERY_PAYLOAD_SIZE = sizeof(int32_t) + NODE_ID_SIZE + DISCOVERY_HOSTNAME_SIZE };

char* discovery_payload_serialize(const struct discovery_payload* payload);
struct discovery_payload discovery_payload_deserialize(const char* buf);

typedef void (*on_device_fn)(
	const struct discovery_payload* payload,
	const struct sockaddr_in* addr
) ;



#endif
