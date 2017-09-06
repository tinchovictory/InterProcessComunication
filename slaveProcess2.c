#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h> // creo que no
#include <sys/select.h> // creo que no
#include <unistd.h>
#include <string.h>
#include <fcntl.h> //creo que no

#include "queue.h"

#define FILENAME_LENGTH 50

T_Queue getFilesFromParent(T_Queue q);



int main(void) {

	/* Create filesQueue */
	T_Queue filesQueue;
	filesQueue = newQueue();

	/* Wait for parent for files */
	filesQueue = getFilesFromParent(filesQueue);

	/* Get hash for every file in filesQueue */
	while(!isEmpty(filesQueue)) {
		char fileName[FILENAME_LENGTH];
		filesQueue = poll(filesQueue, fileName);


		pid_t pid;
		int cpPipe[2]; /* Child - parent pipe */

		/* Create pipe */
		if (pipe(cpPipe) < 0) {
			perror("Child - Parent pipe faild");
			exit(1);
		}


		/* Fork for md5sum */
		pid = fork();

		if(pid < 0) {
	        perror ("Fork faild");
	        exit(1);
		}

		/* Child process code */
		if(pid == 0) {
			/* Configure pipe */

			close(1); /* Close stdout */
			dup(cpPipe[1]); /* Set stdout to write section of pipe */
			close(cpPipe[0]); /* Close unused section of pipe */
			close(cpPipe[1]); /* Close unused section of pipe */
		
			/* Run md5sum */
			execl("/usr/bin/md5sum", "md5sum", fileName, NULL);
				
			close(1);
			exit(0);

		}

		/* Parent process code */

		/* Configure pipe */
		close(cpPipe[1]); //close write end of pipe

		/* Wait for md5sum hash */
		char resp[100] = {0};
		int pos = 0;
		char ch;

		while( read(cpPipe[0], &ch, 1) == 1) {
			resp[pos++] = ch;
		}
		resp[pos] = 0;
		
		/* Write hash to parent */
		dprintf(1, "%s", resp);
		write(1, '\0', 1);
			
	}

	return 0;
}


T_Queue getFilesFromParent(T_Queue q) {
	T_Queue queue = q;
	char fileName[FILENAME_LENGTH] = {0};
	char ch;
	int pos = 0;

	while(read(0, &ch, 1) == 1) {
		


		/* Files separated by \n */
		if(ch == '\n') {
			fileName[pos] = 0;
			queue = offer(queue, fileName);
			//dprintf(1, "acaaa %s",fileName); ///DEBUG
			pos = 0;
		} else {
			fileName[pos++] = ch;
		}


		/* Break on end of read */
		if(ch == '\0') {
			break;
		}
	}
	return queue;
}