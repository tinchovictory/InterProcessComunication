#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>

#include "queue.h"

#define CHILD_PROCESS_Q 5
#define FILE_NAME "hash.txt"

#define FILENAME_LENGTH 200
#define MD5_RESPONSE_LENGTH FILENAME_LENGTH + 50

void createPipe(int pipeFd[CHILD_PROCESS_Q][2], int process);

pid_t createChildProcess();

void runChildProcessCode(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2], int process);

void configureChildPipes(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2], int process);

void configureParentPipes(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]);

int sendFiles(T_Queue filesQueue, int pcPipe[CHILD_PROCESS_Q][2]);

void sendFileToChild(int fd, char * fileName);

void sendEndOfWrite(int pcPipe[CHILD_PROCESS_Q][2]);

void closeWritePipe(int pipeFd[CHILD_PROCESS_Q][2]);

void setNonBlockingRead(int pipeFd[2][2]);

T_Queue readHashFromChilds(int pipeFd[2][2], int filesAmount, char * shm);

T_Queue saveResponse(T_Queue responseQueue, char * str);

T_Queue writeToSharedMemory(T_Queue responseQueue, char * shm);

char * startSHM(char * shm);

int canIWrite(char * shm);

void writeEndOfSHM(char * shm);

void writeToDisk(char * str);

void removeFile();

void formatOutput(char * str);

int isFile(const char *path);
int isDirectory(const char *path);
int isValidDirectory(const char * path);
void formatFileName(char * input, char output[FILENAME_SIZE]);
T_Queue getFiles(int argc, char* argv[]);
void printErrorMsg();


int main(int argc, char* argv[]) {

	pid_t pid;
	T_Queue filesQueue, responseQueue;
	int filesAmount;
	char * shm = NULL;

	/* Get files from input */
	filesQueue = getFiles(argc, argv);

	if(filesQueue == NULL) {
		printf("\n");
		exit(1);
	}

	/* Remove old txt file */
	removeFile();

	/* Pipes arrays */
	int cpPipe[CHILD_PROCESS_Q][2]; /* Child - Parent pipe array */
	int pcPipe[CHILD_PROCESS_Q][2]; /* Parent - Child pipe array */

	int process;

	/* Create CHILD_PROCESS_Q processes */
	for(process = 0; process < CHILD_PROCESS_Q; process++) {
		/* Create child pipe for process */
		createPipe(cpPipe, process);
		createPipe(pcPipe, process);

		/* Create child process */
		pid = createChildProcess();

		/* Child proces code */
		if(pid == 0) {
			runChildProcessCode(pcPipe, cpPipe, process);
		}
	}

	/* Configure parent pipes */
	configureParentPipes(pcPipe, cpPipe);

	/* Send files to child processes */
	filesAmount = sendFiles(filesQueue, pcPipe);

	/* Close write end of pipes after finishing sending files */
	closeWritePipe(pcPipe);

	/* Set read pipe as non-blocking */
	setNonBlockingRead(cpPipe);

	/* Start shared memory for printing the hashes */
	shm = startSHM(shm);

	/* Listen to child process. Returns files not printed by output process */
	responseQueue = readHashFromChilds(cpPipe, filesAmount, shm);

	/* If output process exists */
	if(size(responseQueue) < filesAmount || canIWrite(shm)) {

		/* Print remaining hashes in output process */
		while(!isEmpty(responseQueue)) {
			responseQueue = writeToSharedMemory(responseQueue, shm);
		}

		/* End output process. */
		writeEndOfSHM(shm);
	}

	shmdt(shm);

	return 0;
}


/*
 * Create a one-way pipe for process in parameters.
 */
void createPipe(int pipeFd[CHILD_PROCESS_Q][2], int process) {
	if (pipe(pipeFd[process]) < 0) {
		perror("Child - Parent pipe faild");
		exit(1);
	}
}


/*
 *	Fork a child process.
 *	Returns child process pid.
 */
pid_t createChildProcess() {
	pid_t pid = fork();

	if(pid < 0) {
        perror ("Fork faild");
        exit(1);
	}

	return pid;
}


/*
 *	Code executed by the child process.
 *	Receives two pipes, from parent to child, from child to parent.
 */
void runChildProcessCode(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2], int process) {
	/* Configure child pipes */
	configureChildPipes(pcPipe, cpPipe, process);

	/* Run slave process on child */
	execl("./slaveProcess", "slaveProcess", NULL, NULL);

	/* Run in case of fail */
	close(1);
	exit(1);
}


/*
 *	Configure pipe, so that child process writes as stdout to pipe, and reads as stdin from pipe.
 */
void configureChildPipes(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2], int process) {
	/* Close stdout */
	close(1); 

	/* Set stdout to write section of pipe */
	dup(cpPipe[process][1]);

	/* Close unused side of pipes */
	close(cpPipe[process][0]);
	close(cpPipe[process][1]);

	/* Close stdin */
	close(0);

	/* Set stdin to read section of pipe */
	dup(pcPipe[process][0]);

	/* Close unuesd side of pipes */
	close(pcPipe[process][0]);
	close(pcPipe[process][1]);
}


/*
 *	Configure parent side of pipes.
 */
void configureParentPipes(int pcPipe[CHILD_PROCESS_Q][2], int cpPipe[CHILD_PROCESS_Q][2]) {
	int process;
	for(process = 0; process < CHILD_PROCESS_Q; process++) {
		/* Close read end of pipe */
		close(pcPipe[process][0]);
		/* Close write end of pipe */
		close(cpPipe[process][1]);
	}
}


/*
 * Read files from stdin.
 * Return them as a queue. If there is a non-valid file it returns NULL.
 */
T_Queue getFiles(int argc, char* argv[]) {
	T_Queue filesQueue;
	filesQueue = newQueue();

	int i;
	int valid = 1;

	DIR *mydir;
	struct dirent *myfile;

	char fileName[FILENAME_SIZE];

	for(i = 1; i < argc; i++) {

		if(isDirectory(argv[i]) && isValidDirectory(argv[i])) {
			
			mydir = opendir(argv[i]);
			
			while((myfile = readdir(mydir)) != NULL) {

				if(!((strcmp(myfile->d_name, ".") == 0) || (strcmp(myfile->d_name, "..") == 0))) {
					formatFileName(argv[i], fileName);
					filesQueue = offer(filesQueue, fileName);
				}
			}
			closedir(mydir);
		} else if(isFile(argv[i])) {
			formatFileName(argv[i], fileName);
			filesQueue = offer(filesQueue, fileName);
		} else {
			if(valid) {
				printErrorMsg();
			}
			printf("\t%s\n", argv[i]);
			valid = 0;
		}
	}
	
	if(!valid) {
		return NULL;
	}
	return filesQueue;
}


/*
 * Check if the path is a file.
 */
int isFile(const char *path) {
	struct stat statbuf;
	if(stat(path, &statbuf) != 0) {
	   return 0;
	}
	return S_ISREG(statbuf.st_mode);
}


/*
 * Check if the path is a directory.
 */
int isDirectory(const char *path) {
   struct stat statbuf;
   if(stat(path, &statbuf) != 0) {
	   return 0;
   }
   return S_ISDIR(statbuf.st_mode);
}


/*
 * Check if the directory path ends with '/*'. 
 */
int isValidDirectory(const char * path) {
	int pathLength = strlen(path);
	if(path[pathLength-1] == '*' && path[pathLength-2] == '/') {
		return 1;
	}
	return 0;
}


/*
 * End the file name with a '\n' character. 
 */
void formatFileName(char * input, char output[FILENAME_SIZE]) {
	int i = 0;
	while(*(input+i)) {
		output[i] = input[i];
		i++;
	}

	output[i++] = '\n';
	output[i] = 0;
}


/*
 * Print an error message if a file is not valid.
 */
void printErrorMsg() {
	printf("\n ----- HASH -----\n");
	printf("Parece que ingresaste un archivo o directorio no valido.\n");
	printf("Por favor verifica que los siguientes archivos sean validos:\n");
}


/*
 * Send files for hash to child processes. Distribution is according to the amount of files.
 */
int sendFiles(T_Queue filesQueue, int pcPipe[CHILD_PROCESS_Q][2]) {
	int sentFiles = 0;

	while(!isEmpty(filesQueue)) {
		/* Temporary fileName var */
		char fileName[FILENAME_SIZE] = {0};
		
		/* Get fileName from queue */
		filesQueue = poll(filesQueue,fileName);

		/* Select wich process will hash it. */
		int processNumber = sentFiles % CHILD_PROCESS_Q;

		/* Send file name to process */
		sendFileToChild(pcPipe[processNumber][1], fileName);

		sentFiles++;
	}

	sendEndOfWrite(pcPipe);

	return sentFiles;
}


/*
 * Send file name to child.
 */
void sendFileToChild(int fd, char * fileName) {
	write(fd, fileName, strlen(fileName));
}

/*
 * Send '\0' to each child process, to inform no more writing will be done.
 */
void sendEndOfWrite(int pcPipe[CHILD_PROCESS_Q][2]) {
	int i;
	for(i = 0; i < CHILD_PROCESS_Q; i++) {
		write(pcPipe[i][1], "\0", 1);
	}
}


/*
 * Close write side of pipes.
 */
void closeWritePipe(int pipeFd[CHILD_PROCESS_Q][2]) {
	int i;
	for(i = 0; i < CHILD_PROCESS_Q; i++) {
		close(pipeFd[i][1]);
	}
}


/*
 * Set read pipe as non blocking. This prevents of waiting for a large file while other are ready.
 */
void setNonBlockingRead(int pipeFd[2][2]) {
	int i;
	for(i = 0; i < CHILD_PROCESS_Q; i++) {
		int flags = fcntl(pipeFd[i][0], F_GETFL, 0);
		fcntl(pipeFd[i][0], F_SETFL, flags | O_NONBLOCK);
	}
}


/*
 * Get hashes from child process. Reads iterative from each process.
 * Hashes are saved to a buffer, so shared memory can access it.
 * Hashes are saved to a file.
 * Returns the hashes not printed by hashOutput process.
 */
T_Queue readHashFromChilds(int pipeFd[2][2], int filesAmount, char * shm) {
	T_Queue responseQueue = newQueue();

	char ch;
	int count = 0;

	char fileNameBuff[MD5_RESPONSE_LENGTH] = {0};
	int pos;

	/* Listen to child process until all files hashes are received */
	while(count < filesAmount) {
		int process;

		/* Iterate over every process loking for data */
		for(process = 0; process < CHILD_PROCESS_Q; process++) {
			int resp = read(pipeFd[process][0], &ch, 1);

			/* Chek if I received one character, as read returns -1 if no characters where received. */
			/* Also chek if the received character is an end of read. */
			/* In both cases skip the iteraton. */
			if(resp <= 0 || ch == '\0') {
				continue;
			} else {
				/* Otherwise I save the received character, adn keep reading until '\n' is received. */
				pos = 0;
				fileNameBuff[pos++] = ch;

				while(read(pipeFd[process][0], &ch, 1) > 0 && ch != '\n') {
					fileNameBuff[pos++] = ch;

					if(ch == '\n') {
						/* Full response received. */
						break;
					}
				}

				/* End fileName string */
				fileNameBuff[pos] = 0;

				/* One file was received. */
				count++;

				/* Format response to specified output. */
				formatOutput(fileNameBuff);

				/* Write file to shared memory. */
				responseQueue = saveResponse(responseQueue, fileNameBuff);
			
				/* Write file to disk. */
				writeToDisk(fileNameBuff);
			}

		}
		/* Write file to shared memory */
		responseQueue = writeToSharedMemory(responseQueue, shm);

	}

	return responseQueue;
}



/*
 * If file hash.txt exist, remove it
 */
void removeFile() {
	char * file = FILE_NAME;

	if(access( file, F_OK ) != -1) {
		remove(file);
	}
}


/*
 * Write response to a file on disk.
 */
void writeToDisk(char * str) {
	char * file = FILE_NAME;

	FILE * fpointer;
	fpointer = fopen(file,"a");

	fprintf(fpointer, "%s\n", str);

	fclose(fpointer);

}


/*
 * Add response str to response queue.
 * Retrurns response queue.
 */
T_Queue saveResponse(T_Queue responseQueue, char * str) {
	responseQueue = offer(responseQueue, str);
	return responseQueue;
}


/*
 * Write next response from responseQueue into shared memory with output proceess.
 * Returns updated responseQueue.
 */
T_Queue writeToSharedMemory(T_Queue responseQueue, char * shm) {
	/* If I have hashes to print and output process is ready, I write them in shared memory. */
	if(!isEmpty(responseQueue)) {
		if(canIWrite(shm)) {
			char fileNameBuff[MD5_RESPONSE_LENGTH] = {0};
			responseQueue = poll(responseQueue, fileNameBuff);

			int i;
			for(i = 0; fileNameBuff[i] != 0 && i < MD5_RESPONSE_LENGTH; i++) {
				shm[i] = fileNameBuff[i];
			}
			shm[i] = 0;

		}
	}
	return responseQueue;

}


/*
 * Initialize shared memory.
 * Returns the shared portion of memory.
 */
char * startSHM(char * shm) {
	int shmid;
	key_t key;

	/* Harcoded key */
	key = 1211;

	/* Create a shared memory segment with size MD5_RESPONSE_LENGTH */
	if((shmid = shmget(key, MD5_RESPONSE_LENGTH, IPC_CREAT | 0666)) < 0 ) {
		perror("Shared memory failed\n");
		exit(1);
	}

	/* Atach shared memory to shm pointer */
	if((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
		perror("Shared memory create failed\n");
		exit(1);
	}

	/* Avoid telling the output process that no more hashes to be printed */
	*shm = 's';

	return shm;

}


/*
 * Basic aproach of semaphore.
 * If first character of shared memory is not mofied by output process, I can not write.
 */
int canIWrite(char * shm) {
	if(*shm == '*') {
		return 1;
	}
	return 0;
}


/*
 * Wait until the output process ends reading and write 0 as first character.
 * Output process will know there are no more hashes to be printed. 
 */
void writeEndOfSHM(char * shm) {
	while(1) {
		if(canIWrite(shm)) {
			shm[0] = 0;
			break;
		}
	}
}


/*
 * Turn str to the specified output <file name>: <hash>
 */
void formatOutput(char * str) {
	//printf("%s\n", str);
	char aux[MD5_RESPONSE_LENGTH] = {0};
	int i = 0, j = 0;
	
	while(*(str+i) != ' ') {
		aux[i] = str[i];
		i++;
	}
	aux[i] = 0;

	i += 2;

	while(*(str+i)) {
		str[j++] = str[i++];
	}

	str[j++] = ':';
	str[j++] = ' ';
	i = 0;
	while(aux[i]) {
		str[j++] = aux[i++];
	}
	str[j] = 0;
	//printf("%s\n", str);
}