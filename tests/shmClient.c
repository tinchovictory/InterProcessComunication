#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 100

int canIRead(char * shm);
int isEndOfConnection(char * shm);

int main(void) {
	int shmid;
	char * shm;
	key_t key;

	key = 1211;

	if((shmid = shmget(key, SHM_SIZE, 0666)) < 0 ) {
		printf("Shared memory failed\n");
		exit(1);
	}

	if((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		printf("Shared memory create failed\n");
		exit(1);
	}

	//aviso que llegue

	*shm = '*';

	while(!isEndOfConnection(shm)) {

		if(canIRead(shm)) {
			printf("%s\n", shm);
			*shm = '*';
		}

	}

	return 0;
}

int canIRead(char * shm) {
	if(*shm != '*') {
		return 1;
	}
	return 0;
}

int isEndOfConnection(char * shm) {
	if(*shm == 0) {
		return 1;
	}
	return 0;
}