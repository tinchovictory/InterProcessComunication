#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>

int isFile(const char *path);
int isDirectory(const char *path);
void formatFileName(char * input, char output[100]);
int getFiles(int argc, char* argv[]);

int main(int argc, char* argv[]) {
	if(getFiles(argc, argv) == 0) {
		return 1;
	}
	return 0;
}

int getFiles(int argc, char* argv[]) {
	DIR *mydir;
	struct dirent *myfile;
	struct stat mystat;
	int i;
	int valid = 1;

	char fileName[100];

	for(i = 1; i < argc; i++) {

		if(isDirectory(argv[i])) {
			
			mydir = opendir(argv[i]);
			
			while((myfile = readdir(mydir)) != NULL) {

				if(!((strcmp(myfile->d_name, ".") == 0) || (strcmp(myfile->d_name, "..") == 0))) {
					formatFileName(argv[i], fileName);
					printf("%s", fileName);//offer
				}
			}
			closedir(mydir);
		} else if(isFile(argv[i])) {
			formatFileName(argv[i], fileName);
			printf("%s", fileName);//offer
		} else {
			printf("%s parece que no existe...\n", argv[i]);
			valid = 0;
		}
	}
	return valid;
}

int isFile(const char *path) {
	struct stat statbuf;
	if(stat(path, &statbuf) != 0) {
	   return 0;
	}
	return S_ISREG(statbuf.st_mode);
}

int isDirectory(const char *path) {
   struct stat statbuf;
   if(stat(path, &statbuf) != 0) {
	   return 0;
   }
   return S_ISDIR(statbuf.st_mode);
}

void formatFileName(char * input, char output[100]) {
	int i = 0;
	while(*(input+i)) {
		output[i] = input[i++];
	}

	output[i++] = '\n';
	output[i] = 0;
}