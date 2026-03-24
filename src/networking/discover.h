#ifndef DISCOVERY_H
#define DISCOVERY_H

#include "types.h"
#include <netinet/in.h>

int uid();
int get_listener_socket(int port);
int broadcast(int port, const struct discovery_payload* payload);
int discovery(int dport, int tport, int poll_ms, int uid, const char node_id[NODE_ID_SIZE], on_device_fn);

#endif
