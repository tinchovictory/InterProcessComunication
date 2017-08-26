#include <sys/types.h>
#include <stdio.h>
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

	/* Test files queue */
	filesQueue = offer(filesQueue, "./forkTest.c");
	filesQueue = offer(filesQueue, "./maintest.c");
	filesQueue = offer(filesQueue, "./prog.c");
	/**/
	


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

			/*printf("Pending %d \n", getpid());
			sleep(2);
			printf("Pending %d \n", getpid());
			printf("Pending %d \n", getpid());
			sleep(5);
			printf("Pending %d \n", getpid());
			sleep(8);
			printf("Pending %d \n", getpid());
			sleep(5);*/


			/* Run md5sum in child process */
			execl("/usr/bin/md5sum", "md5sum", fileName, NULL);

			exit(0);

		} else {
			/* Parent process code */

			close(pcPipe[1]); // si no los pongo no anda :(
			close(cpPipe[1]);

			char ch;
			printf("Getting hash...\n");
			while( read(cpPipe[0], &ch, 1) == 1) {
				putchar(ch);
			}

			printf("Done %d\n", pid);


		}


	}
	
	return 0;

}