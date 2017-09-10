#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

/* Node struct, contains a string and a pointer to the next node */
typedef struct node {
	char name[FILENAME_SIZE];
	struct node *next;
} Tnode;

/* Create a new Queue */
T_Queue newQueue() {
	return NULL;
}

/* Add a string to the queue passed by parameters. Returns the new queue. */
T_Queue offer(T_Queue q, const char elem[FILENAME_SIZE]) {
	if(q == NULL) {
		T_Queue aux = malloc(sizeof(Tnode));
		int i;
		for(i = 0; i < FILENAME_SIZE; i++) {
			aux->name[i] = elem[i];
		}
		aux->next = q;
		return aux;
	}
	q->next = offer(q->next, elem);
	return q;
}

/* Remove the first element of the queue. Returns the new queue. The removed item copied to parameter resp */
T_Queue poll(T_Queue q, char resp[FILENAME_SIZE]) {
	if (q == NULL) {
		return NULL;
	}
	int i;
	for(i = 0; i < FILENAME_SIZE; i++) {
		resp[i] = q->name[i];
	}
	return q->next;//falta borrar el otro
}

/* Returns 0 if the queue is empty */
int isEmpty(T_Queue q) {
	if(q == NULL) {
		return 1;
	}
	return 0;
}

/* Returns the queue size */
int size(T_Queue q) {
	if(q == NULL) {
		return 0;
	}
	return 1 + size(q->next);
}