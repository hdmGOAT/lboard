#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
int get_listener_socket(int port);
int broadcast(int port, char msg[]);

int discovery(int port, int poll_ms) {
	int listener = get_listener_socket(port);	
	char discov_msg[] = "CAN YOU CAN YOU CAN YOU FIND ME";

    char buf[1024];
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);

	if (listener != 0){
		perror("listener");
		return 1;
	}

	struct pollfd pfds[1];
	pfds[0].fd =listener;
	pfds[0].events = POLLIN;

	while (1) {
		int ret = poll(pfds, 1, poll_ms);

		if (ret < 0) {
			perror("poll");
			break;
		}

		if (ret == 0) {
			broadcast(port, discov_msg);	
			continue;
		}

        if (pfds[0].revents & POLLIN) {
            ssize_t n = recvfrom(
                listener,
                buf,
                sizeof(buf) - 1,
                0,
                (struct sockaddr *)&src,
                &srclen
            );

            if (n > 0) {
                buf[n] = '\0';
                printf("got '%s' from %s:%d\n",
                       buf,
                       inet_ntoa(src.sin_addr),
                       ntohs(src.sin_port));
            }
        }

	}

	return 0;
}

int get_listener_socket(int port) {
	int listener;
	int yes = 1;

	struct addrinfo hints, *ai, *p;

    char port_str[6];

    snprintf(port_str, sizeof(port_str), "%d", port);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	int gai_status = getaddrinfo(NULL, port_str, &hints, &ai);
	if (gai_status != 0) {
		fprintf(stderr, "get address info: %s \n", gai_strerror(gai_status));
		return -1;
	}

	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

		if (listener < 0) {
			continue;
		}

		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0) {
			perror("setsockopt");
			close(listener);
			continue;
		}

		if((bind(listener, p->ai_addr, p->ai_addrlen)) < 0){
			perror("bind");
			close(listener);
			continue;
		}

		break;
	}

    if (p == NULL) {
        return -1;
    }

    freeaddrinfo(ai);

    return listener;
}

int broadcast(int port, char msg[]) {
    int sockfd;
    struct sockaddr_in broadcast_addr;
    int broadcast_enable = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        perror("setsockopt");
		close(sockfd);	
        return 1;
    }

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

    sendto(sockfd, msg, strlen(msg), 0,
           (struct sockaddr *)&broadcast_addr,
           sizeof(broadcast_addr));

	close(sockfd);	
	return 0;
}
