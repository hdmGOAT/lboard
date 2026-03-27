#include "lboard_app.h"

#include "devices/device_table.h"
#include "discovery/discovery_worker.h"
#include "store/node.h"

#include <stdio.h>
#include <unistd.h>

enum {
    PORT = 5125,
    POLL_MS = 2000,
};

#define TTL_MS (60 * 60 * 0.5)

int lboard_run(void) {
    char node_id[NODE_ID_BYTES];
    if (load_or_create_node_id(node_id) != 0) {
        fprintf(stderr, "failed to load/create node_id\n");
        return 1;
    }

    struct device_table table;
    device_table_init(&table, TTL_MS);

    printf("Discovery listening on UDP %d and broadcasting every %d ms\n", PORT, POLL_MS);
    fflush(stdout);

    if (start_discovery_worker(node_id, &table) != 0) {
        return 1;
    }

    while (1) {
        sleep(5);
        device_table_print(&table);
    }
}
