#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(void) {

	pid_t pid;
	int cpPipe[2][2]; /* Child - parent pipe */

	int process;
	for(process = 0; process < 2; process++) {

		if (pipe(cpPipe[process]) < 0) {
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
			dup(cpPipe[process][1]); /* Set stdout to write section of pipe */

			close(cpPipe[process][0]);
			
			close(cpPipe[process][1]);
			if(process == 0) {
				write(1, "hola soy 0\n", strlen("hola soy 0\n")+1);

				sleep(6);

				//close(1);
				write(1, "hola2 soy 0\n", strlen("hola2 soy 0\n")+1);
			} else {
				sleep(5);

				write(1, "hola soy 1\n", strlen("hola soy 1\n")+1);

				sleep(10);

				//close(1);
				write(1, "hola2 soy 1\n", strlen("hola2 soy 1\n")+1);
			}
			
			close(1);
			

			exit(0);

		} 
		
	}

	/* Parent process code */

	close(cpPipe[0][1]);
	close(cpPipe[1][1]);

	int flags = fcntl(cpPipe[0][0], F_GETFL, 0);
	fcntl(cpPipe[0][0], F_SETFL, flags | O_NONBLOCK);

	flags = fcntl(cpPipe[1][0], F_GETFL, 0);
	fcntl(cpPipe[1][0], F_SETFL, flags | O_NONBLOCK);

	char ch;
	int count = 0;
	printf("Getting hash...\n");
	while(count < 4) { // leo hasta llegar al numero de archivos
		
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
				}
				count++; //aumento el numero de archivos leidos
			}
		}

	}

	printf("\nDone %d\n", pid);

	return 0;
}