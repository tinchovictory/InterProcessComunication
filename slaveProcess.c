#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"


int main(void) {
	
	pid_t pid;
	int pcPipe[2]; /* Parent - child pipe */
	int cpPipe[2]; /* Child - parent pipe */

	char fileName[FILENAME_SIZE]; /* Temporary fileName var */

	/* Create filesQueue */
	T_Queue filesQueue;
	filesQueue = newQueue();
	
	/*Read filesNames sent by parent*/
	char ch;
	int pos = 0;
	while(read(0, &ch, 1) == 1) {

		if(ch != 0 && ch != '\n') {
			fileName[pos++] = ch;
		} else if(ch == '\n') { /* Por alguna razon manda un \n y un 0 al final. Solo me importa el 0*/
			continue;
		} else {
			fileName[pos] = 0;
			filesQueue = offer(filesQueue, fileName);
			pos = 0;
		}
	}

	/* While filesQueue is not empty get hash of files */
	while(!isEmpty(filesQueue)) {
		filesQueue = poll(filesQueue,fileName);

		/* Create pipes */

		if (pipe(pcPipe) < 0) {
			perror("Parent - Child pipe faild");
			exit(1);
		}

		if (pipe(cpPipe) < 0) {
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
			dup(cpPipe[1]); /* Set stdout to write section of pipe */

			close(0); /* Close stdin */
			dup(pcPipe[0]); /* Set stdin to read section of pipe */

			close(cpPipe[0]);
			close(pcPipe[1]);

			/* Run md5sum in child process */
			execl("/usr/bin/md5sum", "md5sum", fileName, NULL);

			exit(0);

		} else {
			/* Parent process code */

			close(pcPipe[1]); /* Close pipe section not beeing used */
			close(cpPipe[1]); /* Close pipe section not beeing used */

			/* Write to parent process md5 hash */
			char ch;
			while( read(cpPipe[0], &ch, 1) == 1) {
				putchar(ch);
			}

		}

	}
	
	return 0;

}