#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define LIMIT 40
#define WIDTH 40

int eraseLastChar(char *input_str, size_t *length){
    if(*length == 0)
        return 0;
    
    (*length)--;
    input_str[*length] = '\0';

    if(write(STDOUT_FILENO, "\b \b", 3) != 3)
        return -1;
    
    return 0;
}

int killLine(char *input_str, size_t *length){
    while(*length > 0){
        if(eraseLastChar(input_str, length) == -1)
            return -1;
    }

    return 0;
}

int eraseLastWord(char *input_str, size_t *length){
    while(*length > 0 && input_str[*length - 1] == ' '){
        if(eraseLastChar(input_str, length) == -1)
            return -1;
    }

    while(*length > 0 && input_str[*length - 1] != ' '){
        if(eraseLastChar(input_str, length) == -1)
            return -1;
    }

    return 0;
}


int main(void){

    char input_str[LIMIT];
    input_str[0] = '\0';
    size_t length = 0;

    struct termios original;
    struct termios modified;

    if(tcgetattr(STDIN_FILENO, &original) == -1){
        perror("tcgetattr");
        exit(1);
    }

    modified = original;

    modified.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);
    modified.c_iflag &= ~IXON;

    modified.c_cc[VMIN] = 1;
    modified.c_cc[VTIME] = 0;

    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &modified) == -1){
        perror("tcsetattr");
        exit(1);
    }

    char ch;
    ssize_t n;

    while((n = read(STDIN_FILENO, &ch, 1)) == 1){
        if(ch == 4){
            if(length == 0)
                break;
            
            write(STDOUT_FILENO, "\a", 1);
            continue;
        }
        if((unsigned char)ch == original.c_cc[VERASE]){
            if(eraseLastChar(input_str, &length) == -1){
                perror("eraseLastChar");
                break;
            }
            continue;
        }
        if((unsigned char)ch == original.c_cc[VKILL]){
            if(killLine(input_str, &length) == -1){
                perror("killLine");
                break;
            }
            continue;
        }
        if((unsigned char)ch == 0x17){
            if(eraseLastWord(input_str, &length) == -1){
                perror("eraseLastWord");
                break;
            }
            continue;
        }
        if(isprint((unsigned char)ch)){
            if(length < LIMIT - 1){
                input_str[length++] = ch;
                input_str[length] = '\0';

                if(write(STDOUT_FILENO, &ch, 1) != 1){
                    perror("write");
                    break;
                }
            }
            else{
                if(write(STDOUT_FILENO, "\a", 1) != 1){
                    perror("write");
                    break;
                }
            }
        }
        else{
            if(write(STDOUT_FILENO, "\a", 1) != 1){
                perror("write");
                break;
            }
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