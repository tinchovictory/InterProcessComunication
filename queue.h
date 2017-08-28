#ifndef __QUEUE
	#define __QUEUE
	
	#define FILENAME_SIZE 50

	typedef struct node * T_Queue;

	/* Create a new Queue */
	T_Queue newQueue();

	/* Add a string to the queue passed by parameters. Returns the new queue. */
	T_Queue offer(T_Queue q, const char elem[FILENAME_SIZE]);

	/* Remove the first element of the queue. Returns the new queue. The removed item copied to parameter resp */
	T_Queue poll(T_Queue q, char resp[FILENAME_SIZE]);

	/* Returns 0 if the queue is empty */
	int isEmpty(T_Queue q);

#endif