#ifndef DISCOVERY_H
#define DISCOVERY_H

int get_listener_socket(int port);
int broadcast(int port, char msg[]);
int discovery(int port, int poll_ms);

#endif
