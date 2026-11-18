#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Usage: %s file...\n", argv[0]);
        return 1;
    }

    int result = 0;

    for(int i=1; i < argc; i++){
        struct stat info;

        if(lstat(argv[i], &info) == -1){
            perror(argv[i]);
            result = 1;
            continue;
        }

        mode_t type = info.st_mode & S_IFMT;
        char file_type = '?';

        if(type == S_IFDIR)
            file_type = 'd';
        else if(type == S_IFREG)
            file_type = '-';
        
        printf("%c %s\n", file_type, argv[i]);
    }

    return result;
}