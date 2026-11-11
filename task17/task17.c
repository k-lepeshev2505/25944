#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define LIMIT 4096
#define WIDTH 40

typedef struct StringNode{
    char *string;
    struct StringNode *next;
} StringNode;


StringNode *createNode(char *input_str){
    StringNode *node = (StringNode*)malloc(sizeof(StringNode));
    if(node == NULL)
        return NULL;

    size_t len = strlen(input_str);
    node->string = (char*)malloc((len+1)*sizeof(char));
    if(node->string == NULL){
        free(node);
        return NULL;
    }

    strcpy(node->string, input_str);
    node->next = NULL;

    return node;
}

void deleteStringList(StringNode *head){
    while(head != NULL){
        StringNode *next = head->next;
        free(head->string);
        free(head);
        head = next;
    }
}


int main(void){

    StringNode *head = NULL;
    StringNode *tail = NULL;

    char input_str[LIMIT];
    size_t length = 0;

    struct termios original;
    struct termios modified;

    if(tcgetattr(STDIN_FILENO, &original) == -1){
        perror("tcgetattr");
        exit(1);
    }

    modified = original;

    modified.c_lflag &= ~(ICANON | ECHO);

    modified.c_cc[VMIN] = 1;
    modified.c_cc[VTIME] = 0;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &modified) == -1){
        perror("tcsetattr");
        exit(1);
    }

    char ch;
    ssize_t n;

    while((n = read(STDIN_FILENO, &ch, 1)) == 1){
        if(ch == 4)
            break;
        if(write(STDOUT_FILENO, &ch, 1) != 1){
            perror("write");
            break;
        }
    }

    if(n == -1)
        perror("read");

    if(tcsetattr(STDIN_FILENO, TCSANOW, &original) == -1){
        perror("tcsetattr restore");
        exit(1);
    }

    exit(0);
}