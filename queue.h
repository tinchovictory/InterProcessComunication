#ifndef __QUEUE
	#define __QUEUE
	
	#define FILENAME_SIZE 50

	typedef struct node * T_Queue;


	T_Queue newQueue();

	T_Queue offer(T_Queue q, const char elem[FILENAME_SIZE]);

	T_Queue poll(T_Queue q, char resp[FILENAME_SIZE]);

	int isEmpty(T_Queue q);

#endif