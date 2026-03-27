#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <unistd.h>
struct list_head {
	struct list_head *prev, *next;
};


static inline void INIT_LIST_HEAD(struct list_head *list){
	list->next = list;
	list->prev = list;
}

static inline void list_add(struct list_head *added, struct list_head *head){
	added->prev = head->prev;
	added->next = head;
	head->prev->next = added;
	head->prev = added;
}

static inline void list_del(struct list_head *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;

    node->next = NULL;
    node->prev = NULL;
}

#endif
