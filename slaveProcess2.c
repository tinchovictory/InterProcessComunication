#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "queue.h"

#define FILENAME_LENGTH 50
#define MD5_RESPONSE_LENGTH FILENAME_LENGTH + 50

T_Queue getFilesFromParent();

void createPipe(int pipeFd[2]);

pid_t createChildProcess();

void runChildCode(int cpPipe[2], char * fileName);

void configureChildPipes(int cpPipe[2]);

void getMd5Response(int fd, char resp[MD5_RESPONSE_LENGTH]);

void sendToParent(char msg[MD5_RESPONSE_LENGTH]);

int main(void) {

	/* Create filesQueue */
	T_Queue filesQueue;

	/* Wait for parent for files */
	filesQueue = getFilesFromParent();

	/* Get hash for every file in filesQueue */
	while(!isEmpty(filesQueue)) {
		/* Get file name to hash into fileName var */
		char fileName[FILENAME_LENGTH];
		filesQueue = poll(filesQueue, fileName);

		pid_t pid;
		int cpPipe[2]; /* Child - parent pipe */

		/* Create pipe */
		createPipe(cpPipe);

		/* Fork for md5sum */
		pid = createChildProcess();

		/* Child process code */
		if(pid == 0) {
			runChildCode(cpPipe, fileName);
		}

		/* Parent process code */

		/* Configure parent side of pipe. Close write end of pipe. */
		close(cpPipe[1]);

		/* Wait for md5sum hash */
		char md5ResponseBuff[MD5_RESPONSE_LENGTH] = {0};
		getMd5Response(cpPipe[0], md5ResponseBuff);

		/* Write hash to parent */
		sendToParent(md5ResponseBuff);
			
	}

	return 0;
}


/* 
 *	Read files from stdin(parent), and save them into a queue.
 *	Returns the queue.
 *	Files recived must be separated by '\n' character.
 *	End of files is sent as '\0'.
 */
T_Queue getFilesFromParent() {
	T_Queue q = newQueue();

	char fileName[FILENAME_LENGTH] = {0}; /* Buffer for reading chars */
	char ch;
	int pos = 0;

	/* Read one char at the time until end or '\0' is received. Files are separated by '\n' */
	while(read(0, &ch, 1) == 1) {
		
		/* End of file received. Add the buffer as string to the queue */
		if(ch == '\n') {
			fileName[pos] = 0;
			q = offer(q, fileName);
			pos = 0;
		} else {
			fileName[pos++] = ch; /* Add character to buffer if not '\n' */
		}

		/* Break on end of read */
		if(ch == '\0') {
			break;
		}
	}
	return q;
}

/*
 * Create a one-way pipe.
*/
void createPipe(int pipeFd[2]) {
	if(pipe(pipeFd) < 0) {
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
 *	Receives a one-way pipe from child to parent, and the file name to hash.
 */
void runChildCode(int cpPipe[2], char * fileName) {
	/* Configure pipe */
	configureChildPipes(cpPipe);

	/* Run md5sum */
	execl("/usr/bin/md5sum", "md5sum", fileName, NULL);
	
	/* Run in case of fail */	
	close(1);
	exit(0);
}

/*
 *	Configure pipe, so that child process writes as stdout to pipe.
 */
void configureChildPipes(int cpPipe[2]) {
	/* Close stdout */
	close(1); 

	/* Set stdout to write section of pipe */
	dup(cpPipe[1]); 
	
	/* Close unused sections of pipe */
	close(cpPipe[0]); 
	close(cpPipe[1]);
}

/*
 *	Read file descriptor and set response to resp.
 *	File descriptor should be the child to parent pipe, so it reads md5sum output.
 */
void getMd5Response(int fd, char resp[MD5_RESPONSE_LENGTH]) {
	int pos = 0;
	char ch;

	while( read(fd, &ch, 1) == 1) {
		resp[pos++] = ch;
	}
	resp[pos] = 0;
}

/*
 *	Send msg to stdout (as we are in a pipe, stdout is parent)
 */
void sendToParent(char msg[MD5_RESPONSE_LENGTH]) {
	/* Printf didn't work :( */
	dprintf(1, "%s", msg);

	/* Send end of write to parent. */
	write(1, '\0', 1);
}