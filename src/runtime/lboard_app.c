#include "lboard_app.h"

#include "devices/device_table.h"
#include "discovery/discovery_worker.h"
#include "store/node.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

enum {
    PORT = 5125,
    POLL_MS = 2000,
};

#define TTL_MS (60 * 60 * 0.5)

static volatile sig_atomic_t shutdown_requested = 0;

static void on_shutdown_signal(int signum) {
    (void)signum;
    shutdown_requested = 1;
}

static int install_signal_handlers(void) {
    struct sigaction action;

    action.sa_handler = on_shutdown_signal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) != 0) {
        perror("sigaction SIGINT");
        return -1;
    }

    if (sigaction(SIGTERM, &action, NULL) != 0) {
        perror("sigaction SIGTERM");
        return -1;
    }

    return 0;
}

int lboard_run(void) {
    char node_id[NODE_ID_BYTES];
    if (load_or_create_node_id(node_id) != 0) {
        fprintf(stderr, "failed to load/create node_id\n");
        return 1;
    }

    struct device_table table;
    device_table_init(&table, TTL_MS);

    if (install_signal_handlers() != 0) {
        device_table_destroy(&table);
        return 1;
    }

    printf("Discovery listening on UDP %d and broadcasting every %d ms\n", PORT, POLL_MS);
    fflush(stdout);

    if (start_discovery_worker(node_id, &table) != 0) {
        device_table_destroy(&table);
        return 1;
    }

    while (!shutdown_requested) {
        device_table_expire_devices(&table);
        device_table_print(&table);
        sleep(5);
    }

    if (stop_discovery_worker() != 0) {
        device_table_destroy(&table);
        return 1;
    }

    device_table_destroy(&table);
    return 0;
}
