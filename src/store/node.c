#include "node.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void get_node_id_path(char *out, size_t len) {
    const char *base = getenv("XDG_CONFIG_HOME");
    if (!base) {
        const char *home = getenv("HOME");
        if (!home) {
            fprintf(stderr, "No HOME set\n");
            exit(1);
        }
        snprintf(out, len, "%s/.config/lboard", home);
    } else {
        snprintf(out, len, "%s/lboard", base);
    }

    mkdir(out, 0700);
    strncat(out, "/node_id", len - strlen(out) - 1);
}

int load_or_create_node_id(char out[NODE_ID_BYTES]) {
    char path[512];
    get_node_id_path(path, sizeof(path));

    FILE *f = fopen(path, "rb");
    if (f) {
        size_t n = fread(out, 1, NODE_ID_BYTES, f);
        fclose(f);
        return n == NODE_ID_BYTES ? 0 : -1;
    }

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
        return -1;

    if (read(fd, out, NODE_ID_BYTES) != NODE_ID_BYTES) {
        close(fd);
        return -1;
    }
    close(fd);

    f = fopen(path, "wb");
    if (!f)
        return -1;

    chmod(path, 0600);
    fwrite(out, 1, NODE_ID_BYTES, f);
    fclose(f);

    return 0;
}
