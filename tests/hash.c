#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "queue.h"

#define CHILD_PROCESS_Q 5
#define FILES_AMOUNT 7


void createChildProcesess(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]);
void createPipes(int process, int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]);
void configureChildProcess(int process, int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]);
T_Queue getFiles();
void sendFiles(T_Queue filesQueue, int pcPipe[CHILD_PROCESS_Q][2]);
void sendFileToChild(int pcPipe[CHILD_PROCESS_Q][2], int process, char fileName[FILENAME_SIZE]);
void sendEndToChilds(int pcPipe[CHILD_PROCESS_Q][2]);
void closePipes(int pipe[CHILD_PROCESS_Q][2], int fd);
void setNonBlockingRead(int pipe[CHILD_PROCESS_Q][2]);


int main(void) {


	/* CHILD_PROCESS_Q pipes arrays */
	int pcPipe[CHILD_PROCESS_Q][2];
	int cpPipe[CHILD_PROCESS_Q][2];

	/* Files queue */
	T_Queue filesQueue;

	/* Create CHILD_PROCESS_Q procecess */
	createChildProcesess(pcPipe, cpPipe);
	
	/* Close pipe section not beeing used. */
	closePipes(pcPipe,0);
	closePipes(cpPipe,1);

	/* Get files from input */
	filesQueue = getFiles();

	/* Send files to child process */
	sendFiles(filesQueue, pcPipe);

	/* Close pipe after writing */
	closePipes(pcPipe,1);

	/* Set read pipes as non-blocking */
	setNonBlockingRead(cpPipe);

	/* Listen to child process */

	int files = FILES_AMOUNT;
	char ch;
	int count = 0;
	while(count < files) { // leo hasta llegar al numero de archivos
		
		int i;
		for(i = 0; i < 2; i++) { //itero por los pipes buscando cual tiene datos para leer
			int resp = read(cpPipe[i][0], &ch, 1);

			if(resp < 0) {//da -1 si no tiene nada para leer (habria que verificar error especifico)
				continue;
			} else if(resp == 0) { //se cerro el pipe del otro lado, lo salteo
				break;
			} else { //guardo el contenido de lo que leo
				printf("From Process %d:\n", i);
				putchar(ch);
				while(read(cpPipe[i][0], &ch, 1) > 0) { //sigo leyendo hasta el final
					putchar(ch);
					if(ch == '\n') {
						break;
					}
				}
				count++; //aumento el numero de archivos leidos
			}
		}

	}


	printf("\nDone!!\n");


	return 0;
}

void createChildProcesess(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]) {
	int process;
	for(process = 0; process < CHILD_PROCESS_Q; process++) {
		
		createPipes(process, pcPipe, cpPipe);

		/* Create child process */
		int pid = fork();

		if(pid < 0) {
			/* Fork faild! */
	        perror ("Fork faild");
	        exit(1);
		}

		if(pid == 0) {
			/* Child process code */
			
			configureChildProcess(process, pcPipe, cpPipe);
			
			/* Run slave process in child process */
			execl("./slaveProcess", "slaveProcess", NULL, NULL);
			//execl("./tests/slaveTest", "slaveTest", NULL, NULL);

			close(1);
			exit(0);

		}
	}
}

void createPipes(int process, int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]) {
	if (pipe(pcPipe[process]) < 0) {
		perror("Parent - Child pipe faild");
		exit(1);
	}

	if (pipe(cpPipe[process]) < 0) {
		perror("Child - Parent pipe faild");
		exit(1);
	}
}

void configureChildProcess(int process, int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]) {
	close(1); /* Close stdout */
	dup(cpPipe[process][1]); /* Set stdout to write section of pipe */

	close(0); /* Close stdin */
	dup(pcPipe[process][0]); /* Set stdin to read section of pipe */

	close(cpPipe[process][0]); /* Close pipe section not beeing used */
	close(cpPipe[process][1]); /* Close pipe section not beeing used */
	close(pcPipe[process][0]); /* Close pipe section not beeing used */
	close(pcPipe[process][1]); /* Close pipe section not beeing used */
}

T_Queue getFiles() {
	T_Queue filesQueue;
	filesQueue = newQueue();

	filesQueue = offer(filesQueue, "./tests/forkTest.c");
	filesQueue = offer(filesQueue, "./tests/Movie.mkv");
	filesQueue = offer(filesQueue, "./tests/forkTest.c");
	filesQueue = offer(filesQueue, "./tests/forkTest.c");
	filesQueue = offer(filesQueue, "./tests/Movie.mkv");
	filesQueue = offer(filesQueue, "./tests/forkTest.c");
	
	filesQueue = offer(filesQueue, "./tests/maintest.c");

	return filesQueue;
}

void sendFiles(T_Queue filesQueue, int pcPipe[CHILD_PROCESS_Q][2]) {
	int count = 0;
	while(!isEmpty(filesQueue)) {
		char fileName[FILENAME_SIZE]; /* Temporary fileName var */
		filesQueue = poll(filesQueue,fileName);

		int processNumber = count % CHILD_PROCESS_Q;
		count++;

		sendFileToChild(pcPipe, processNumber, fileName);
	}
	sendEndToChilds(pcPipe);
}

void sendFileToChild(int pcPipe[CHILD_PROCESS_Q][2], int process, char fileName[FILENAME_SIZE]) {
	write(pcPipe[process][1], fileName, strlen(fileName));
}

void sendEndToChilds(int pcPipe[CHILD_PROCESS_Q][2]) {
	int i;
	for(i = 0; i < CHILD_PROCESS_Q; i++) {
		write(pcPipe[i][1], '\0', 1);
	}
}

void closePipes(int pipe[CHILD_PROCESS_Q][2], int fd) {
	int process;
	for(process = 0; process < CHILD_PROCESS_Q; process++) {
		close(pipe[process][fd]);
	}
}

void setNonBlockingRead(int pipe[CHILD_PROCESS_Q][2]) {
	int process;
	for(process = 0; process < CHILD_PROCESS_Q; process++) {
		int flags = fcntl(pipe[process][0], F_GETFL, 0);
		fcntl(pipe[process][0], F_SETFL, flags | O_NONBLOCK);
	}
}
