#include "types.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

char* discovery_payload_serialize(const struct discovery_payload* payload) {
    char* buf = malloc(DISCOVERY_PAYLOAD_SIZE);
    if (buf == NULL) {
        return NULL;
    }

    int32_t net_uid = htonl(payload->uid);
    int32_t net_port = htonl(payload->port);

    memcpy(buf, &net_uid, sizeof(int32_t));
    memcpy(buf + sizeof(int32_t), &net_port, sizeof(int32_t));
	memcpy(buf + sizeof(int32_t) * 2, payload->node_id, NODE_ID_SIZE);
    memcpy(buf + sizeof(int32_t) * 2 + NODE_ID_SIZE, payload->hostname, DISCOVERY_HOSTNAME_SIZE);

    return buf;
}

struct discovery_payload discovery_payload_deserialize(const char* buf) {
    struct discovery_payload payload;

    int32_t net_uid;
    int32_t net_port;

    memcpy(&net_uid, buf, sizeof(int32_t));
    memcpy(&net_port, buf + sizeof(int32_t), sizeof(int32_t));
    memcpy(payload.node_id, buf + sizeof(int32_t) * 2, NODE_ID_SIZE);
    memcpy(payload.hostname, buf + sizeof(int32_t) * 2 + NODE_ID_SIZE, DISCOVERY_HOSTNAME_SIZE);
    payload.hostname[DISCOVERY_HOSTNAME_SIZE - 1] = '\0';

    payload.uid = ntohl(net_uid);
    payload.port = ntohl(net_port);

    return payload;
}
