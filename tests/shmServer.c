#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_SIZE 100

int canIWrite(char * shm);

int main(void) {
	int shmid;
	char * shm;
	key_t key;

	key = 1211;

	if((shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666)) < 0 ) {
		printf("Shared memory failed\n");
		exit(1);
	}

	if((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		printf("Shared memory create failed\n");
		exit(1);
	}


	int i;
	for(i = 0; i < 5; ) {
		if(canIWrite(shm)) {
			/* Escribo en shm */
			shm[0] = 'h';
			shm[1] = 'o';
			shm[2] = 'l';
			shm[3] = 'a';
			shm[4] = 0;
			//fprintf(shm, "%s\n", "hola");
			i++;
		}
	}

	//end of write
	while(1) {
		if(canIWrite(shm)) {
			shm[0] = 0;
			break;
		}
	}

	return 0;
}

int canIWrite(char * shm) {
	if(*shm == '*') {
		return 1;
	}
	return 0;
}


/*

agrego las respuestas a una lista
cuando pueda escribir escribo la primera
si no puedo escribir sigo

si ya escribi una en main sigo escribiendo
espero hasta que se escriban todas

si no esctibi nada termino el proceso




leeo la respuesta
agrego a una lista

llamo a funcion
funcion(lista):
	si la lista no esta vacia
		si puedo esctibir
			saco de la lista y lo escribo 
		

puedo saber si hay algun cliente?

*/