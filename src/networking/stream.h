#ifndef STREAM_H
#define STREAM_H

#include "networking/types.h"
#include <stddef.h>
#include <sys/poll.h>

int connect_tcp_socket(const char *ip, int port);
int get_tcp_listener(int port, int backlog);
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);
int process_connections(struct pollfd **pfds, int *fd_count, int *fd_size, int listener, on_client_payload handle_payload);
int poll_tcp(int port, int backlog, int fdsize, on_client_payload handle_payload);

#endif
