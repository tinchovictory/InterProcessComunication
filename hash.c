#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

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

		close(cpPipe[0]); /* Close pipe section not beeing used */
		close(pcPipe[1]); /* Close pipe section not beeing used */

		/* Run slave process in child process */
		execl("./slaveProcess", "slaveProcess", NULL);

		exit(0);

	} else {
		/* Parent process code */

		close(cpPipe[1]); /* Close pipe section not beeing used */

		/* TEST */ //CAMBAIAR por la lista de archivos
		write(pcPipe[1], "./tests/forkTest.c", sizeof("./tests/forkTest.c"));
		write(pcPipe[1], "./tests/maintest.c", sizeof("./tests/maintest.c"));
		write(pcPipe[1], "./tests/forkTest.c", sizeof("./tests/forkTest.c"));
		/**/

		close(pcPipe[1]); /* Close write pipe after writing */

		/* Read hash from slave */ //CAMBIAR por funcion que guarda la palabra en un buffer
		char ch;
		while( read(cpPipe[0], &ch, 1) == 1) {
			putchar(ch);
		}

		printf("Done %d\n", pid);

	}

	return 0;
}