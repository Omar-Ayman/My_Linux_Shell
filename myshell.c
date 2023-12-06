
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

typedef enum ExecutionMode {
    Normal,
    ToFile,
} ExecutionMode;

#define COMMAND_BUFFER_SIZE 256
#define WRITE_BUFFER_SIZE 1024

int main() {
    char ws[] = " \n\r\t";
    char line[COMMAND_BUFFER_SIZE], *tokens[COMMAND_BUFFER_SIZE >> 1], **token, *output, writeBuffer[WRITE_BUFFER_SIZE + 1];
    ExecutionMode mode;
    int p[2], bufferCount;
    FILE *fd;

    writeBuffer[WRITE_BUFFER_SIZE] = 0;

    while (1) {
        printf("=> ");
        if (!fgets(line, COMMAND_BUFFER_SIZE, stdin)) {
            printf("couldn't read line:\n");
            return -1;
        }

        token = tokens;
        if (!(*token = strtok(line, ws)))
            continue;

        mode = Normal;

        while (*++token = strtok(NULL, ws)) {
            if (!strcmp(*token, ">")) {
                *token = NULL;
                output = strtok(NULL, ws);
                mode = ToFile;
                break;
            }
        }

        if (mode == Normal) {
            if (!fork()) {
                execvp(tokens[0], tokens);
                printf("%s was not recognized as a command\n", tokens[0]);
                return 1;
            } else {
                wait(0);
            }
        } else {
            pipe(p);
            if (mode == ToFile) {
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
                    if (!(fd = fopen(output, "w"))) {
                        printf("could not write in file %s\n", output);
                        return 3;
                    }
                    while (fgets(writeBuffer, WRITE_BUFFER_SIZE, stdin)) {
                        fprintf(fd, "%s", writeBuffer);
                    }

                    fclose(fd);
                    return 0;
                }

                close(p[0]);
                close(p[1]);
                wait(0);
                wait(0);
            }
        }
    }
}
