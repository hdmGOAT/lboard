#include "discover.h"
#include "types.h"
#include <stdlib.h>
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

int discovery(int dport, int tport, int poll_ms, const char node_id[NODE_ID_SIZE], on_device_fn on_device) {
	int listener = get_listener_socket(dport);
	struct discovery_payload payload = {
		.port = tport,
	};
	memcpy(payload.node_id, node_id, NODE_ID_SIZE);
	memset(payload.hostname, 0, sizeof(payload.hostname));
	if (gethostname(payload.hostname, sizeof(payload.hostname)) != 0) {
		snprintf(payload.hostname, sizeof(payload.hostname), "unknown");
	}
    char buf[DISCOVERY_PAYLOAD_SIZE];
    struct sockaddr_in src;
    socklen_t srclen = sizeof(src);

	if (listener < 0){
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
			broadcast(dport, &payload);	
			continue;
		}

        if (pfds[0].revents & POLLIN) {
            ssize_t n = recvfrom(
                listener,
                buf,
				sizeof(buf),
                0,
                (struct sockaddr *)&src,
                &srclen
            );

			if (n == DISCOVERY_PAYLOAD_SIZE) {
				struct discovery_payload received_payload = discovery_payload_deserialize(buf);
				if (memcmp(received_payload.node_id, node_id, NODE_ID_SIZE) == 0) {
					continue;
				}

				on_device(&received_payload, &src);
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

int broadcast(int port, const struct discovery_payload* payload) {
    int sockfd;
    struct sockaddr_in broadcast_addr;
    int broadcast_enable = 1;
	char* msg = discovery_payload_serialize(payload);

	if (msg == NULL) {
		return 1;
	}

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	
    if (sockfd < 0) {
        perror("socket");
		free(msg);
        return 1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        perror("setsockopt");
		close(sockfd);	
		free(msg);
        return 1;
    }

    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(port);
    broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

	sendto(sockfd, msg, DISCOVERY_PAYLOAD_SIZE, 0,
           (struct sockaddr *)&broadcast_addr,
           sizeof(broadcast_addr));

	free(msg);
	close(sockfd);	
	return 0;
}
