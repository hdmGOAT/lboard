#ifndef NODE_H
#define NODE_H

enum { NODE_ID_BYTES = 32 };

int load_or_create_node_id(char out[NODE_ID_BYTES]);

#endif
