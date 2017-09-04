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


void createChildProcesess(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]);
void createPipes(int process, int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]);
void configureChildProcess(int process, int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]);
T_Queue getFiles();
void sendFiles(T_Queue filesQueue, int pcPipe[CHILD_PROCESS_Q][2]);
void sendFileToChild(int pcPipe[CHILD_PROCESS_Q][2], int process, char fileName[FILENAME_SIZE]);
void closePipes(int pipe[CHILD_PROCESS_Q][2]);



int main(void) {


	/* CHILD_PROCESS_Q pipes arrays */
	int pcPipe[CHILD_PROCESS_Q][2];
	int cpPipe[CHILD_PROCESS_Q][2];

	/* Files queue */
	T_Queue filesQueue;

	/* Create CHILD_PROCESS_Q procecess */
	createChildProcesess(pcPipe, cpPipe);
	
	/* Close pipe section not beeing used */
	closePipes(cpPipe);

	/* Get files from input */
	filesQueue = getFiles();

	/* Send files to child process */
	sendFiles(filesQueue, pcPipe);

	/* Close pipe after writing */
	closePipes(pcPipe);

	/* Set read pipes as non-blocking */
	//setNonBlockingRead(cpPipe);

	int j;
	for(j = 0; j < CHILD_PROCESS_Q; j++) {
		int flags = fcntl(cpPipe[j][0], F_GETFL, 0);
		fcntl(cpPipe[j][0], F_SETFL, flags | O_NONBLOCK);
	}

	/* Listen to child process */

	char ch;
	int count = 0;
	//printf("Getting hash...\n");
	while(count < 7) { // leo hasta llegar al numero de archivos
		
		int i;
		for(i = 0; i < CHILD_PROCESS_Q; i++) { //itero por los pipes buscando cual tiene datos para leer
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


	/*char ch;
	fd_set readSet;
	int filesCount = 0;
	while(filesCount < 6) {
		FD_ZERO(&readSet);
		int j, maxfd;
		for (j=0; j<CHILD_PROCESS_Q; j++) {
		   FD_SET(cpPipe[j][0], &readSet);
		   maxfd = (maxfd>cpPipe[j][0])?maxfd:cpPipe[j][0];
		}
		// Now, check for readability
		int result = select(maxfd+1, &readSet, NULL, NULL, NULL);

		//printf("%d\n", result);
		printf("%d\n", FD_ISSET(cpPipe[0][0], &readSet));
		printf("%d\n", FD_ISSET(cpPipe[1][0], &readSet));
		printf("%d\n", FD_ISSET(cpPipe[2][0], &readSet));
		printf("%d\n", FD_ISSET(cpPipe[3][0], &readSet));
		printf("%d\n", FD_ISSET(cpPipe[4][0], &readSet));


		if(result > 0) {
			for (j=0; j<CHILD_PROCESS_Q; j++) {
			    if (FD_ISSET(cpPipe[j][0], &readSet)) {
			    	printf("From process %d: ", j);
			    	filesCount++;
			        while( read(cpPipe[j][0], &ch, 1) == 1) {
						putchar(ch);
						if(ch == '\n') {
							break;
						}
					}
			    }
			}
		}
	}*/
/*
	fd_set readSet;
	int i, highfd = 0, fileCount = 0;
	char ch;
	while(fileCount < 7) {
		FD_ZERO(&readSet);
		for (i = 0; i < CHILD_PROCESS_Q; i++) {
	        FD_SET(cpPipe[i][0], &readSet);
	        if(cpPipe[i][0] > highfd) {
	        	highfd = cpPipe[i][0];
	        }
	    }

	    int returned = select(highfd + 1, &readSet, NULL, NULL, NULL);

	    if (returned > 0) {
	        for (i = 0; i < CHILD_PROCESS_Q; i++) {
	            if (FD_ISSET(cpPipe[i][0], &readSet)) {
	                //printf("Process %d has data\n", i);
	                while( read(cpPipe[i][0], &ch, 1) == 1) {
	                	putchar(ch);
	                	if(ch == '\n') {
	                		printf("process %d\n", i);
	                		fileCount++;
							break;
						}
	                }
	                
	            }
	        }
	    }
	}

*/	

	
/*


	char ch;
	int filesCount = 0;
	while(filesCount < 6) {
		int i;
		for(i = 0; i < CHILD_PROCESS_Q; i++) {
			
			if(read(cpPipe[i][0], &ch, 1) == 1 && ch != 0 && ch != '\n') { // si leyo algo sigo leyendo
				printf("From process %d: ", i);	
					putchar(ch);
					filesCount++;
				
				while( read(cpPipe[i][0], &ch, 1) == 1) {
					putchar(ch);
					if(ch == '\n') {
						break;
					}
				}			
			}
		}
	}
*/
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
			execl("./slaveProcess", "slaveProcess", NULL);

			exit(0);

		}
		//printf("Process %d pid: %d\n",process, pid );
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
}

void sendFileToChild(int pcPipe[CHILD_PROCESS_Q][2], int process, char fileName[FILENAME_SIZE]) {
	write(pcPipe[process][1], fileName, strlen(fileName) + 1);
}

void closePipes(int pipe[CHILD_PROCESS_Q][2]) {
	int process;
	for(process = 0; process < CHILD_PROCESS_Q; process++) {
		close(pipe[process][1]);
	}
}





/**********************************************/
int Oldmain() {

	pid_t pid;
	int pcPipe[1][2]; /* Parent - child pipe */
	int cpPipe[1][2]; /* Child - parent pipe */

	/* Create pipes */

	if (pipe(pcPipe[0]) < 0) {
		perror("Parent - Child pipe faild");
		exit(1);
	}

	if (pipe(cpPipe[0]) < 0) {
		perror("Child - Parent pipe faild");
		exit(1);
	}
	

	/* Create child process */
	pid = fork();

	if(pid < 0) {
		/* Fork faild! */
        perror ("Fork faild");
        exit(1);
	}

	if(pid == 0) {
		/* Child process code */
		
		close(1); /* Close stdout */
		dup(cpPipe[0][1]); /* Set stdout to write section of pipe */

		close(0); /* Close stdin */
		dup(pcPipe[0][0]); /* Set stdin to read section of pipe */

		close(cpPipe[0][0]); /* Close pipe section not beeing used */
		close(pcPipe[0][1]); /* Close pipe section not beeing used */

		/* Run slave process in child process */
		execl("./slaveProcess", "slaveProcess", NULL);

		exit(0);

	} else {
		/* Parent process code */

		close(cpPipe[0][1]); /* Close pipe section not beeing used */

		/* TEST */ //CAMBAIAR por la lista de archivos
		write(pcPipe[0][1], "./tests/forkTest.c", sizeof("./tests/forkTest.c"));
		write(pcPipe[0][1], "./tests/maintest.c", sizeof("./tests/maintest.c"));
		write(pcPipe[0][1], "./tests/forkTest.c", sizeof("./tests/forkTest.c"));
		/**/

		close(pcPipe[0][1]); /* Close write pipe after writing */

		/* Read hash from slave */ //CAMBIAR por funcion que guarda la palabra en un buffer
		char ch;
		while( read(cpPipe[0][0], &ch, 1) == 1) {
			putchar(ch);
		}

		printf("Done %d\n", pid);

	}

	return 0;
}