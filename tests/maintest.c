#include <stdio.h>
#include "queue.h"

int main(void) {
	T_Queue q;
	q = newQueue();
	q = offer(q, "Hola");
	q = offer(q, "Chau");
	q = offer(q, "End");

	char msg[FILENAME_SIZE];

	while(!isEmpty(q)) {
		q = poll(q,msg);
		printf("%s\n", msg);
	}

	return 0;
}
