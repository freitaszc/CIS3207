#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

/**
 * Splits the command string into an array of arguments.
 * @param cmd The command line string.
 * @param args The array to store the tokenized arguments.
 */
void parse_command(char *cmd, char **args) {
    int i = 0;
    char *token = strtok(cmd, " \t\n");

    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL; // Null-terminate the argument list
}

int main() {
    char cmd[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    pid_t pid;
    int status;

    while (1) {
        // 1. Print Prompt
        printf("myshell> ");
        fflush(stdout); // Ensure prompt is displayed immediately

        // 2. Read Input
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            printf("\n"); // Handle Ctrl+D (EOF) elegantly
            break;
        }

        // 3. Parse Arguments
        parse_command(cmd, args);

        // Handle empty commands (user just pressed Enter)
        if (args[0] == NULL) {
            continue;
        }

        // Handle built-in "exit" command
        if (strcmp(args[0], "exit") == 0) {
            printf("Exiting shell...\n");
            break;
        }

        // 4. Fork and Execute
        pid = fork();

        if (pid < 0) {
            // Error handling for fork failure
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            // --- Child Process ---
            // execvp searches the PATH for the command automatically
            if (execvp(args[0], args) < 0) {
                perror("Execution failed");
            }
            exit(1); // Exit child if execvp fails
        } else {
            // --- Parent Process ---
            // Wait for the child process to finish
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}