
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main() {
    char line[256], *tokens[128], **token;

    while (1) {
        printf("=> ");
        if (!gets(line)) {
            printf("couldn't read line:\n");
            return -1;
        }

        token = tokens;
        if (!(*token = strtok(line, " \t")))
            continue;

        while (*++token = strtok(NULL, " \t"));

        if (!fork()) {
            execvp(tokens[0], tokens);
            printf("%s was not recognized as a command\n", tokens[0]);
            return 1;
        } else {
            wait(0);
        }
    }
}
