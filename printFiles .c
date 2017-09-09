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

int main(int argc, char* argv[])
{
    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;

    for(int i = 1; i < argc; i++){
        if(isDirectory(argv[i])){
            mydir = opendir(argv[i]);
            while((myfile = readdir(mydir)) != NULL){
                if (!((strcmp(myfile->d_name, ".") == 0) || (strcmp(myfile->d_name, "..") == 0))){
                    printf(" %s\n", myfile->d_name);
                }
            }
            closedir(mydir);
        }else if(isFile(argv[i])){
        printf("%s\n", basename(argv[i]));
        }
    }
    return 0;
}

int isFile(const char *path){
    struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISREG(statbuf.st_mode);
}

int isDirectory(const char *path){
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}