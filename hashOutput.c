#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define FILENAME_LENGTH 200
#define MD5_RESPONSE_LENGTH FILENAME_LENGTH + 50

void welcomeMsg();
void endMsg();
char * startSHM(char * shm);
int canIRead(char * shm);
int isEndOfConnection(char * shm);
void sendImReady(char *  shm);

int main(void) {
	char * shm = NULL;

	/* Start shared memory. */
	shm = startSHM(shm);

	welcomeMsg();

	/* Read from shared memory until end character received. */
	while(!isEndOfConnection(shm)) {
		/* Only read if is my turn. */
		if(canIRead(shm) && !isEndOfConnection(shm)) {
			
			printf("%s\n", shm);
			
			sendImReady(shm);
		}

	}

	endMsg();

	return 0;
}


void welcomeMsg() {
	printf("\n --- Welcome to HASH ---\n\n");
	printf("Searching for hashes...\n\n");
}

void endMsg() {
	printf("\n\nThanks for using --- HASH ---\n\n");
}


/*
 * Start shared memory.
 */
char * startSHM(char * shm) {
	int shmid;
	key_t key;

	/* Harcoded key */
	key = 1211;

	/* Create a shared memory segment with size MD5_RESPONSE_LENGTH */
	if((shmid = shmget(key, MD5_RESPONSE_LENGTH, 0666)) < 0 ) {
		printf("Shared memory failed\n");
		exit(1);
	}

	/* Atach shared memory to shm pointer */
	if((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		printf("Shared memory create failed\n");
		exit(1);
	}

	/* Inform main process that I was created */
	sendImReady(shm);

	return shm;
}


/*
 * Check if I can read.
 * I can read when the first character of shm is not '*'
 */
int canIRead(char * shm) {
	if(*shm != '*') {
		return 1;
	}
	return 0;
}


/*
 * Check for end of connection.
 */
int isEndOfConnection(char * shm) {
	if(*shm == 0) {
		return 1;
	}
	return 0;
}


/*
 * Inform main process that I'm ready to read.
 */
void sendImReady(char *  shm) {
	*shm = '*';
}