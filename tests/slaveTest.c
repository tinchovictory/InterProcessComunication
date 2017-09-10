#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(void) {

	char c;
	int r = 0;
/*	do {
		r = read(0,&c,1);
		write(1, &c, 1);
	} while(r == 1 && c != '\0');
*/
	int files = 0;
	char msg1[100] = {0};
	while(read(0,&c,1) == 1) {
		//write(1, &c, 1);
		msg1[r++] = c;
		if(c == '\0') {
			break;
		}
		if(c == '\n') {
			files++;
		}
	}

	dprintf(1, "%s", msg1);

	dprintf(1, "Termine de leer\n");

for(r=0;r<files;r++) {
	pid_t pid;
	int cpPipe[2]; /* Child - parent pipe */

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
		close(cpPipe[0]);
		close(cpPipe[1]);
	
		execl("/usr/bin/md5sum", "md5sum", "./Movie.mkv", NULL);
			
		close(1);
		exit(0);

	}


	/* Parent process code */

	close(cpPipe[1]); //close write end of pipe

	dprintf(1,"Hashing...\n");

	/*int flags = fcntl(cpPipe[0], F_GETFL, 0);
	fcntl(cpPipe[0], F_SETFL, flags | O_NONBLOCK);
	*/
	char msg[100] = {0};
	int pos = 0;
	char ch;

	/*while(1) {
		int resp = read(cpPipe[0], &ch, 1);
		if(resp < 0) {//da -1 si no tiene nada para leer (habria que verificar error especifico)
			continue;
		} else if(resp == 0) { //se cerro el pipe del otro lado, lo salteo
			break;
		} else { //guardo el contenido de lo que leo
			msg[pos++] = ch;
			//putchar(ch);
			while(read(cpPipe[0], &ch, 1) > 0) { //sigo leyendo hasta el final
				//putchar(ch);
				msg[pos++] = ch;
			}
		}
	}*/

	//dprintf(1,"hash\n");


	while( read(cpPipe[0], &ch, 1) == 1) {
		msg[pos++] = ch;
	}
	msg[pos] = 0;
	dprintf(1, "%s", msg);
	write(1, '\0', 1);
	//sleep(2);

}
	dprintf(1,"done\n");


	return 0;
}
