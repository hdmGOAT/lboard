#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "stream.h"
int get_tcp_listener(int port, int backlog) {
	struct addrinfo hints, *res, *p;
	int sockfd = -1;
	int gai_status, bind_status, listen_status;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

	gai_status = getaddrinfo(NULL, port_str, &hints, &res);
	if (gai_status != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_status));
		return -1;
	}

	for (p = res; p != NULL; p= p->ai_next){
		sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sockfd == -1) {
			perror("socket");
			continue;
		}

		bind_status = bind(sockfd, p->ai_addr, p->ai_addrlen);
		if (bind_status != -1){
			break;
		}

		close(sockfd);
		sockfd = -1;
	}

	if (p == NULL) {
		perror("bind");
		freeaddrinfo(res);
		if (sockfd != -1) {
			close(sockfd);
		}
		return -1;
	}

	listen_status = listen(sockfd, backlog);
	if (listen_status < 0) {
		perror("listen failed");
		freeaddrinfo(res);
		close(sockfd);
		return -1;
	}

	freeaddrinfo(res);

	return sockfd;
}

void add_to_pfds(struct pollfd **pfds, int newfd, int *fd_count, int *fd_size){
	if (*fd_count == *fd_size){
		*fd_size *= 2;
		*pfds = realloc(*pfds, sizeof(**pfds) *(*fd_size));
	}

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN;
    (*pfds)[*fd_count].revents = 0;

    (*fd_count)++;
}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    pfds[i] = pfds[*fd_count-1];

    (*fd_count)--;
}

int handle_client_payload(int client_fd, const char *buf, size_t len)
{
	(void)client_fd;
	(void)buf;
	(void)len;
	return 0;
}

int process_connections(struct pollfd **pfds, int *fd_count, int *fd_size, int listener)
{
	int i;
	for (i = 0; i < *fd_count; i++) {
		if (((*pfds)[i].revents & POLLIN) == 0) {
			continue;
		}

		if ((*pfds)[i].fd == listener) {
			struct sockaddr_storage remoteaddr;
			socklen_t addrlen = sizeof(remoteaddr);
			int newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
			if (newfd == -1) {
				perror("accept");
				continue;
			}

			add_to_pfds(pfds, newfd, fd_count, fd_size);
			continue;
		}

		char buf[1024];
		ssize_t nbytes = recv((*pfds)[i].fd, buf, sizeof(buf), 0);
		if (nbytes <= 0) {
			if (nbytes < 0) {
				perror("recv");
			}

			close((*pfds)[i].fd);
			del_from_pfds(*pfds, i, fd_count);
			i--;
			continue;
		}

		if (handle_client_payload((*pfds)[i].fd, buf, (size_t)nbytes) != 0) {
			close((*pfds)[i].fd);
			del_from_pfds(*pfds, i, fd_count);
			i--;
		}
	}

	return 0;
}

int poll_tcp(int port, int backlog, int fdsize){
	int listener = get_tcp_listener(port, backlog);
	if (listener < 0) {
		perror("error setting up listener");
		return -1;
	}

	int fd_count = 0;

	struct pollfd *pfds = malloc(sizeof(*pfds) * fdsize);
	if (pfds == NULL) {
		perror("malloc");
		close(listener);
		return -1;
	}

	pfds[0].fd = listener;
	pfds[0].events = POLLIN;
	fd_count++;
	
	for(;;){
		int poll_count = poll(pfds, fd_count, -1);
		if (poll_count == -1) {
			perror("poll");
			free(pfds);
			close(listener);
			return -1;
		}

		if (poll_count == 0) {
			continue;
		}

		if (process_connections(&pfds, &fd_count, &fdsize, listener) != 0) {
			free(pfds);
			close(listener);
			return -1;
		}
	}
}
