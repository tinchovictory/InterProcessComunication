#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	
	pid_t pid;
	int pcPipe[2]; /* Parent - child pipe */
	int cpPipe[2]; /* Child - parent pipe */


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
		//failure in creating a child
        perror ("Fork faild");
        exit(1);
	}

	if(pid == 0) {
		// only child process
		
		close(1); /* Close stdout */
		dup(cpPipe[1]); /* Set stdout to write section of pipe */

		close(0); /* Close stdin */
		dup(pcPipe[0]); /* Set stdin to read section of pipe */

		close(cpPipe[0]);
		close(pcPipe[1]);

		//write(1, "Im child process", 17);
		//execl("./prog","prog",NULL);

		execl("/usr/bin/md5sum","md5sum", "./forkTest.c",NULL);

		exit(0);

	} else {
		// only parent process

		close(pcPipe[1]); // si no los pongo no anda :(
		close(cpPipe[1]);

		char ch;
		printf("Waiting for child...\n");
		while( read(cpPipe[0], &ch, 1) == 1) {
			putchar(ch);
		}

		

	}


	return 0;

}