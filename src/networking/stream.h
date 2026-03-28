#ifndef STREAM_H
#define STREAM_H

#include <stddef.h>
#include <sys/poll.h>

int get_tcp_listener(int port, int backlog);
void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size);
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);
int handle_client_payload(int client_fd, const char *buf, size_t len);
int process_connections(struct pollfd **pfds, int *fd_count, int *fd_size, int listener);
int poll_tcp(int port, int backlog, int fdsize);

#endif
