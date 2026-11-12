#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

#define LIMIT 4096
#define WIDTH 40

void eraseLastChar(char *input_str, size_t *length){
    if(*length == 0)
        return;
    
    (*length)--;
    input_str[*length] = '\0';
    
}

void killLine(char *input_str, size_t *length){
    while(*length > 0)
        eraseLastChar(input_str, length);

}

void eraseLastWord(char *input_str, size_t *length){
    while(*length > 0 && input_str[*length - 1] == ' ')
        eraseLastChar(input_str, length);

    while(*length > 0 && input_str[*length - 1] != ' ')
        eraseLastChar(input_str, length);
}

int drawWrapped(const char *text, size_t length, size_t* rows){
    size_t i = 0, column = 0, end = 0, word_end = 0;

    *rows = 1;

    while(i < length){
        end = i;

        if(text[i] == ' '){
            while(end < length && text[end] == ' ')
                end++;
            
            word_end = end;

            while(word_end < length && text[word_end] != ' ')
                word_end++;

            if(word_end > end && column > 0 && column + (word_end - i) > WIDTH){
                if(write(STDOUT_FILENO, "\r\n", 2) != 2)
                    return -1;
                
                (*rows)++;
                column = 0;
                i = end;
                continue;
            }
        }
        else{
            while(end < length && text[end] != ' ')
                end++;
            
            if(column > 0 && column + (end - i) > WIDTH){
                if(write(STDOUT_FILENO, "\r\n", 2) != 2)
                    return -1;

                (*rows)++;
                column = 0;
            }
        }

        while(i < end){
            if(column == WIDTH){
                if(write(STDOUT_FILENO, "\r\n", 2) != 2)
                    return -1;
                
                (*rows)++;
                column = 0;
            }

            if(write(STDOUT_FILENO, &text[i], 1) != 1)
                return -1;
            
            column++;
            i++;
        }
    }

    return 0;
}

int redrawLine(const char *text, size_t length, size_t *previous_rows){
    char command[32];
    int n = 0;

    if(write(STDOUT_FILENO, "\r", 1) != 1)
        return -1;
    
    if(*previous_rows > 1){
        n = snprintf(command, sizeof(command), "\033[%zuA", *previous_rows - 1);

        if(n < 0 || (size_t)n >= sizeof(command))
            return -1;
        
        if(write(STDOUT_FILENO, command, (size_t)n) != n)
            return -1;
    }

    if(write(STDOUT_FILENO, "\033[J", 3) != 3)
        return -1;

    size_t new_rows;

    if(drawWrapped(text, length, &new_rows) == -1)
        return -1;

    *previous_rows = new_rows;

    return 0;
}


int main(void){

    char input_str[LIMIT];
    input_str[0] = '\0';
    size_t length = 0, displayed_rows = 1;

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
            eraseLastChar(input_str, &length);

            if(redrawLine(input_str, length, &displayed_rows) == -1){
                perror("redrawLine");
                break;
            }
            continue;
        }
        if((unsigned char)ch == original.c_cc[VKILL]){
            killLine(input_str, &length);

            if(redrawLine(input_str, length, &displayed_rows) == -1){
                perror("redrawLine");
                break;
            }
            continue;
        }
        if((unsigned char)ch == 0x17){
            eraseLastWord(input_str, &length);

            if(redrawLine(input_str, length, &displayed_rows) == -1){
                perror("redrawLine");
                break;
            }
            continue;
        }
        if(isprint((unsigned char)ch)){
            if(length < LIMIT - 1){
                input_str[length++] = ch;
                input_str[length] = '\0';

                if(redrawLine(input_str, length, &displayed_rows) == -1){
                    perror("redrawLine");
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