#ifndef DISCOVERY_H
#define DISCOVERY_H

#include "types.h"
#include <netinet/in.h>

typedef int (*should_stop_fn)(void *ctx);

int get_listener_socket(int port);
int broadcast(int port, const struct discovery_payload* payload);
int discovery(int dport, int tport, int poll_ms,
			  const char node_id[NODE_ID_SIZE],
			  on_device_fn on_device,
			  void *ctx,
			  should_stop_fn should_stop,
			  void *stop_ctx);

#endif
