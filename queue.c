#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

//#define FILENAME_SIZE 50

//typedef struct node * T_Queue;

typedef struct node {
	char name[FILENAME_SIZE];
	struct node *next;
} Tnode;

T_Queue newQueue() {
	return NULL;
}

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

int isEmpty(T_Queue q) {
	if(q == NULL) {
		return 1;
	}
	return 0;
}
