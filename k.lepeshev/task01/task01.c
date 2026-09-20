#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>
#include <ulimit.h>

typedef struct Option{
    char name;
    char *argument;
} Option;


int main(int argc, char *argv[]){

    size_t capacity = 1;

    for(int i=1; i < argc; i++)
        capacity += strlen(argv[i]);

    Option *options = (Option*)malloc(capacity*sizeof(Option));

    if(options == NULL){
        perror("malloc");
        return 1;
    }


    int idx = 0;

    int option = 0;

    while((option = getopt(argc, argv, "ipdsuU:")) != -1){
        switch(option){
            case 'i':
            case 'p':
            case 'd':
            case 's':
            case 'u':
                options[idx].name = option;
                options[idx].argument = NULL;
                idx++;
                break;
            case 'U':
                options[idx].name = option;
                options[idx].argument = optarg;
                idx++;
                break;
            case '?':
                printf("Unknown option\n");
                break;

        }
    }

    for(int i=idx-1; i >= 0; i--){
        switch(options[i].name){
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
            case 's':
                if(setpgid(0, 0) == -1)
                    perror("setpgid");
                break;
            case 'u':
            {
                long limit = ulimit(UL_GETFSIZE);

                if(limit == -1)
                    perror("ulimit");
                else
                    printf("File size limit: %ld\n", limit);

                break;
            }
            case 'U':
            {
                char *endptr;
                
                errno = 0;

                long new_limit = strtol(options[i].argument, &endptr, 10);

                if(endptr == options[i].argument || *endptr != '\0' || errno == ERANGE || new_limit < 0){
                    printf("Invalid ulimit value\n");
                    break;
                }

                long result = ulimit(UL_SETFSIZE, new_limit);

                if(result == -1)
                    perror("ulimit");

                break;
            }

        }
    }

    free(options);

    return 0;
}