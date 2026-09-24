#include <stdio.h>
#include <unistd.h> // для getuid()
#include <stdlib.h>


int main(){

    printf("Real UID: %lu\n", (unsigned long)getuid());
    printf("Effective UID: %lu\n", (unsigned long)geteuid());

    FILE *file_in = fopen("input.txt", "r");

    if(file_in == NULL){
        perror("open");
    }

    fclose(file_in);

    if(setuid(0) == -1){
        perror("setuid");
    }

    printf("Real UID: %lu\n", (unsigned long)getuid());
    printf("Effective UID: %lu\n", (unsigned long)geteuid());

    FILE *file_in = fopen("input.txt", "r");

    if(file_in == NULL){
        perror("open");
    }

    fclose(file_in);


    exit(0);
}