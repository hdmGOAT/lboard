#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <stdint.h>

enum { DISCOVERY_HOSTNAME_SIZE = 64 };

struct discovery_payload {
    int32_t uid;
    int32_t port;
    char hostname[DISCOVERY_HOSTNAME_SIZE];
};

enum { DISCOVERY_PAYLOAD_SIZE = sizeof(int32_t) * 2 + DISCOVERY_HOSTNAME_SIZE };

char* discovery_payload_serialize(const struct discovery_payload* payload);
struct discovery_payload discovery_payload_deserialize(const char* buf);

#endif
