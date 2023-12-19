
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define debug(S, ...) {printf("????? ");printf(S __VA_OPT__(,) __VA_ARGS__);printf("\n");}
#define return_err(ERR) if (ERR != -1) return ERR;

typedef enum ExecutionMode {
    Exit = 0,
    WriteToFile = 1,
    AppendToFile = 2,
    Normal = 3,
    Pipe,
} ExecutionMode;

#define COMMAND_BUFFER_SIZE 256
#define WRITE_BUFFER_SIZE 1024
#define DIR_BUFFER_SIZE 256
#define WS " \n\r\t"

int exec_cmd(char **tokens, char *writeBuffer) {
    ExecutionMode mode = Normal;
    int p[2], bufferCount, err;
    FILE *fd;
    char const fileModes[][3] = {"w+", "a+"};
    char *output, **token = tokens;

    while (*++token = strtok(NULL, WS)) {
        if ((*token)[0] == '>') {
            if ((*token)[1] == '>')
                mode = AppendToFile;
            else
                mode = WriteToFile;
            *token = NULL;
            output = strtok(NULL, WS);
            break;
        } else if (!strcmp(*token, "|")) {
            mode = Pipe;
            *token = NULL;
            break;
        }
    }

    if (mode == Normal) {
        if (!strcmp(tokens[0], "cd")) {
            if (chdir(tokens[1])) {
                printf("couldn't find a directory with the name \"%s\"\n", tokens[1]);
            }
        } else if (!fork()) {
            execvp(tokens[0], tokens);
            printf("%s was not recognized as a command\n", tokens[0]);
            return 1;
        } else {
            wait(0);
        }
    } else {
        pipe(p);
        if (!fork()) {
            dup2(p[1], 1);
            close(p[0]);
            execvp(tokens[0], tokens);
            printf("%s was not recognized as a command\n", tokens[0]);
            return 2;
        }
        
        if (!fork()) {
            dup2(p[0], 0);
            close(p[1]);
            if (mode == WriteToFile || mode == AppendToFile) {
                if (!(fd = fopen(output, fileModes[mode - WriteToFile]))) {
                    printf("could not write in file %s\n", output);
                    return 3;
                }
                while (fgets(writeBuffer, WRITE_BUFFER_SIZE, stdin)) {
                    fprintf(fd, "%s", writeBuffer);
                }

                fclose(fd);
            }

            if (mode == Pipe) {
                *tokens = strtok(NULL, WS);
                err = exec_cmd(tokens, writeBuffer);
                return_err(err);
            }

            return 0;
        }

        close(p[0]);
        close(p[1]);
        wait(0);
        wait(0);
    }

    return -1;
}

int main() {
    int err;
    char line[COMMAND_BUFFER_SIZE], *tokens[COMMAND_BUFFER_SIZE >> 1], writeBuffer[WRITE_BUFFER_SIZE + 1], dir[DIR_BUFFER_SIZE];
    
    writeBuffer[WRITE_BUFFER_SIZE] = 0;

    while (1) {
        printf("%s => ", getcwd(dir, DIR_BUFFER_SIZE));
        if (!fgets(line, COMMAND_BUFFER_SIZE, stdin)) {
            printf("couldn't read line:\n");
            return -1;
        }

        if (!(*tokens = strtok(line, WS)))
            continue;

        err = exec_cmd(tokens, writeBuffer);
        return_err(err);
    }
}
