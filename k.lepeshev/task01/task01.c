#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


int main(int argc, char *argv[]){

    size_t capacity = 1;

    for(int i=1; i < argc; i++)
        capacity += strlen(argv[i]);

    char *options = (char*)malloc(capacity);
    int idx = 0;

    int option = 0;

    while((option = getopt(argc, argv, "ipd")) != -1){
        switch(option){
            case 'i':
            case 'p':
            case 'd':
                options[idx++] = (char)option;
                break;
            case '?':
                printf("Unknown option\n");
                break;

        }
    }
    for(int i=idx-1; i >= 0; i--){
        switch(options[i]){
            case 'i':
                printf("Real UID: %lu\n", (unsigned long)getuid());
                printf("Effective UID: %lu\n", (unsigned long)geteuid());
                printf("Real GID: %lu\n", (unsigned long)getgid());
                printf("Effective GID: %lu\n", (unsigned long)getegid());
                break;
            case 'p':
                printf("Process ID: %ld\n", (long)getpid());
                printf("Parent process ID: %ld\n", (long)getppid());
                printf("Group process ID: %ld\n", (long)getpgrp());
                break;
            case 'd':
            {
                char path[4096];

                if(getcwd(path, sizeof(path)) != NULL)
                    printf("Current directory: %s\n", path);
                else
                    perror("getcwd");

                break;
            }
        }
    }

    free(options);

    return 0;
}