#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	

	
	int pipes[2];


	//create a pipe
	if(pipe(pipes) < 0) {
		//failure in creating a pipe
        perror("pipe");
        exit (1);
	}

	pid_t pid = fork();

	if(pid < 0) {
		//failure in creating a child
        perror ("fork");
        exit(2);
	}

	if(pid == 0) {
		// only child process
		printf("Im the child process, pid=%d\n",getpid() );

		//execl("/Users/martin/Documents/ITBA/SO/prog","prog",NULL);
		// execl("./prog","prog",NULL);

		char message[64] = "Hola soy el mensaje del child process";

          //while(1) {
                    //Clearing the message
              //      memset (message, 0, sizeof(message));
              //      printf ("Enter a message: ");
              //      scanf ("%s",message);

                    //Writing message to the pipe
                    write(pipes[1], message, strlen(message));
            //}
            exit (0);

	} else {
		// only parent process
		printf("Im the parent process, pid=%d, and the child pid is %d\n",getpid(), pid );
		//wait(NULL); //wait until child process is done
		//printf("Im done\n");


		char message[64];

         while (1) {
                    //Clearing the message buffer
                    memset (message, 0, sizeof(message));

                    //Reading message from the pipe
                   read (pipes[0], message, sizeof(message));
                    printf("Message entered %s\n",message);
            }

            exit(0);
	}



	return 0;
}