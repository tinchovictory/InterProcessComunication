#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "queue.h"

#define CHILD_PROCESS_Q 5

void createPipe(int pipeFd[CHILD_PROCESS_Q][2], int process);

pid_t createChildProcess();

void runChildProcessCode(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2], int process);

void configureChildPipes(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2], int process);

void configureParentPipes(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]);

T_Queue getFiles();

int sendFiles(T_Queue filesQueue, int pcPipe[CHILD_PROCESS_Q][2]);

void sendFileToChild(int fd, char * fileName);

void sendEndOfWrite(int pcPipe[CHILD_PROCESS_Q][2]);

void closeWritePipe(int pipeFd[CHILD_PROCESS_Q][2]);

void setNonBlockingRead(int pipeFd[2][2]);

void readHashFromChilds(int pipeFd[2][2], int filesAmount);



int main(void) {

	pid_t pid;
	T_Queue filesQueue;
	int filesAmount;

	/* Pipes arrays */
	int cpPipe[CHILD_PROCESS_Q][2]; /* Child - Parent pipe array */
	int pcPipe[CHILD_PROCESS_Q][2]; /* Parent - Child pipe array */

	int process;

	/* Create CHILD_PROCESS_Q processes */
	for(process = 0; process < CHILD_PROCESS_Q; process++) {
		/* Create child pipe for process */
		createPipe(cpPipe, process);
		createPipe(pcPipe, process);

		/* Create child process */
		pid = createChildProcess();

		/* Child proces code */
		if(pid == 0) {
			runChildProcessCode(pcPipe, cpPipe, process);
		}
	}

	/* Configure parent pipes */
	configureParentPipes(pcPipe, cpPipe);

	/* Get files from input */
	filesQueue = getFiles();

	/* Send files to child processes */
	filesAmount = sendFiles(filesQueue, pcPipe);

	/* Close write end of pipes after finishing sending files */
	closeWritePipe(pcPipe);

	/* Set read pipe as non-blocking */
	setNonBlockingRead(cpPipe);

	/* Listen to child process */
	readHashFromChilds(cpPipe, filesAmount);

	return 0;
}


/*
 * Create a one-way pipe for process in parameters.
 */
void createPipe(int pipeFd[CHILD_PROCESS_Q][2], int process) {
	if (pipe(pipeFd[process]) < 0) {
		perror("Child - Parent pipe faild");
		exit(1);
	}
}


/*
 *	Fork a child process.
 *	Returns child process pid.
 */
pid_t createChildProcess() {
	pid_t pid = fork();

	if(pid < 0) {
        perror ("Fork faild");
        exit(1);
	}

	return pid;
}


/*
 *	Code executed by the child process.
 *	Receives two pipes, from parent to child, from child to parent.
 */
void runChildProcessCode(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2], int process) {
	/* Configure child pipes */
	configureChildPipes(pcPipe, cpPipe, process);

	/* Run slave process on child */
	execl("./slaveProcess2", "slaveProcess2", NULL, NULL);

	/* Run in case of fail */
	close(1);
	exit(0);
}


/*
 *	Configure pipe, so that child process writes as stdout to pipe, and reads as stdin from pipe.
 */
void configureChildPipes(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2], int process) {
	/* Close stdout */
	close(1); 

	/* Set stdout to write section of pipe */
	dup(cpPipe[process][1]);

	/* Close unused side of pipes */
	close(cpPipe[process][0]);
	close(cpPipe[process][1]);

	/* Close stdin */
	close(0);

	/* Set stdin to read section of pipe */
	dup(pcPipe[process][0]);

	/* Close unuesd side of pipes */
	close(pcPipe[process][0]);
	close(pcPipe[process][1]);
}


/*
 *	Configure parent side of pipes.
 */
void configureParentPipes(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]) {
	int process;
	for(process = 0; process < CHILD_PROCESS_Q; process++) {
		/* Close read end of pipe */
		close(pcPipe[process][0]);
		/* Close write end of pipe */
		close(cpPipe[process][1]);
	}
}


/*
 * Read files from stdin.
 * Return them as a queue.
 */
T_Queue getFiles() {
	T_Queue filesQueue;
	filesQueue = newQueue();


	filesQueue = offer(filesQueue, "./tests/forkTest.c\n");
	filesQueue = offer(filesQueue, "./tests/Movie.mkv\n");
	filesQueue = offer(filesQueue, "./tests/maintest.c\n");

	filesQueue = offer(filesQueue, "./tests/forkTest.c\n");
	
	filesQueue = offer(filesQueue, "./tests/forkTest.c\n");
	filesQueue = offer(filesQueue, "./tests/Movie.mkv\n");
	filesQueue = offer(filesQueue, "./tests/maintest.c\n");
	

	return filesQueue;
}


/*
 * Send files for hash to child processes. Distribution is according to the amount of files.
 */
int sendFiles(T_Queue filesQueue, int pcPipe[CHILD_PROCESS_Q][2]) {
	int sentFiles = 0;

	while(!isEmpty(filesQueue)) {
		/* Temporary fileName var */
		char fileName[FILENAME_SIZE] = {0};
		
		/* Get fileName from queue */
		filesQueue = poll(filesQueue,fileName);

		/* Select wich process will hash it. */
		int processNumber = sentFiles % CHILD_PROCESS_Q;

		/* Send file name to process */
		sendFileToChild(pcPipe[processNumber][1], fileName);

		sentFiles++;
	}

	sendEndOfWrite(pcPipe);

	return sentFiles;
}


/*
 * Send file name to child.
 */
void sendFileToChild(int fd, char * fileName) {
	write(fd, fileName, strlen(fileName));
}

/*
 * Send '\0' to each child process, to inform no more writing will be done.
 */
void sendEndOfWrite(int pcPipe[CHILD_PROCESS_Q][2]) {
	int i;
	for(i = 0; i < CHILD_PROCESS_Q; i++) {
		write(pcPipe[i][1], "\0", 1);
	}
}


/*
 * Close write side of pipes.
 */
void closeWritePipe(int pipeFd[CHILD_PROCESS_Q][2]) {
	int i;
	for(i = 0; i < CHILD_PROCESS_Q; i++) {
		close(pipeFd[i][1]);
	}
}


/*
 * Set read pipe as non blocking. This prevents of waiting for a large file while other are ready.
 */
void setNonBlockingRead(int pipeFd[2][2]) {
	int i;
	for(i = 0; i < CHILD_PROCESS_Q; i++) {
		int flags = fcntl(pipeFd[i][0], F_GETFL, 0);
		fcntl(pipeFd[i][0], F_SETFL, flags | O_NONBLOCK);
	}
}


/*
 * Get hashes from child process. Reads iterative from each process.
 * Hashes are saved to a buffer, so shared memory can access it.
 * Hashes are saved to a file.
 */
void readHashFromChilds(int pipeFd[2][2], int filesAmount) {
	/* Write child process data to shared memory */
	/* Write child process data to file */

	char ch;
	int count = 0;
	while(count < filesAmount) { // leo hasta llegar al numero de archivos
		
		int i;
		for(i = 0; i < CHILD_PROCESS_Q; i++) { //itero por los pipes buscando cual tiene datos para leer
			int resp = read(pipeFd[i][0], &ch, 1);
			if(resp <= 0 || ch == '\0') {//da -1 si no tiene nada para leer (habria que verificar error especifico)
				continue;
			} else { //guardo el contenido de lo que leo
				printf("From Process %d:\n", i);
				putchar(ch);
				while(read(pipeFd[i][0], &ch, 1) > 0 && ch != '\n') { //sigo leyendo hasta el final
					putchar(ch);
					if(ch == '\n') {
						break;
					}
				}
				putchar('\n');
				count++; //aumento el numero de archivos leidos
			}
		}

	}
}
