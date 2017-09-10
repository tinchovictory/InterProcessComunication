#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define FILES_AMOUNT 3
#define CHILD_PROCESS_Q 2

void readData(int cpPipe[2][2]);
void setNonBlockingRead(int cpPipe[2][2]);
void closePipe(int pipe[2], int fd);

int main(void) {

	pid_t pid;
	int cpPipe[CHILD_PROCESS_Q][2]; /* Child - parent pipe */
	int pcPipe[CHILD_PROCESS_Q][2]; /* Parent - child pipe */

	int process;
	for(process = 0; process < CHILD_PROCESS_Q; process++) {

		if (pipe(cpPipe[process]) < 0) {
			perror("Child - Parent pipe faild");
			exit(1);
		}
		if (pipe(pcPipe[process]) < 0) {
			perror("Parent - Child pipe faild");
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
			dup(cpPipe[process][1]); /* Set stdout to write section of pipe */
			close(cpPipe[process][0]);
			close(cpPipe[process][1]);

			close(0); /* Close stdin */
			dup(pcPipe[process][0]);
			close(pcPipe[process][0]);
			close(pcPipe[process][1]);

			execl("./slaveProcess2", "slaveProcess2", NULL, NULL);
			//execl("./slaveTest", "slaveTest", NULL, NULL);

			printf("fail\n");
			if(process == 1) {
				//exit(0);
				//execl("./slaveTest", "slaveTest", NULL, NULL);

			} else {
				//execl("/usr/bin/md5sum", "md5sum", "./pipes.c", NULL);
				//execl("./slaveTest", "slaveTest", NULL, NULL);
			}
			
			close(1);
			exit(0);

		}
	}

	/* Parent process code */

	closePipe(pcPipe[0],0); // close read end of pipe
	closePipe(pcPipe[1],0);

	closePipe(cpPipe[1],1);
	closePipe(cpPipe[0],1); // close write end of pipes

	write(pcPipe[0][1], "./tests/forkTest.c\n", strlen("./tests/forkTest.c\n"));

	write(pcPipe[1][1], "./hash.c\n", strlen("./hash.c\n")+1);

	

	

	write(pcPipe[0][1], "./tests/Movie.mkv\n", strlen("./tests/Movie.mkv\n")+1);
	
	closePipe(pcPipe[1],1);//no quiero ecribir el 1
	closePipe(pcPipe[0],1); //ya no quiero enviar mas

	setNonBlockingRead(cpPipe);	
	
	printf("Getting hash...\n");
	
	readData(cpPipe);

	printf("\nDone %d\n", pid);

	return 0;
}

void closePipe(int pipe[2], int fd) {
	close(pipe[fd]);
}

void setNonBlockingRead(int cpPipe[2][2]) {
	int flags = fcntl(cpPipe[0][0], F_GETFL, 0);
	fcntl(cpPipe[0][0], F_SETFL, flags | O_NONBLOCK);

	flags = fcntl(cpPipe[1][0], F_GETFL, 0);
	fcntl(cpPipe[1][0], F_SETFL, flags | O_NONBLOCK);
}

void readData(int cpPipe[2][2]) {
	int files = FILES_AMOUNT;
	char ch;
	int count = 0;
	while(count < files) { // leo hasta llegar al numero de archivos
		
		int i;
		for(i = 0; i < CHILD_PROCESS_Q; i++) { //itero por los pipes buscando cual tiene datos para leer
			int resp = read(cpPipe[i][0], &ch, 1);
			if(resp < 0) {//da -1 si no tiene nada para leer (habria que verificar error especifico)
				continue;
			} else if(resp == 0) { //se cerro el pipe del otro lado, lo salteo
				continue;
			} else if(ch == '\0') {
				continue;
			} else { //guardo el contenido de lo que leo
				printf("From Process %d:\n", i);
				putchar(ch);
				while(read(cpPipe[i][0], &ch, 1) > 0 && ch != '\n') { //sigo leyendo hasta el final
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