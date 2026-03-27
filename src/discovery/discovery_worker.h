#ifndef DISCOVERY_WORKER_H
#define DISCOVERY_WORKER_H

#include "networking/types.h"
#include "store/node.h"

int start_discovery_worker(
    const char node_id[NODE_ID_BYTES],
    struct device_table *table
);
int stop_discovery_worker(void);

#endif
