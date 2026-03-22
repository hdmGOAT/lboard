#ifndef DISCOVERY_H
#define DISCOVERY_H

#include "types.h"

int uid();
int get_listener_socket(int port);
int broadcast(int port, const struct discovery_payload* payload);
int discovery(int port, int poll_ms, int uid);

#endif
