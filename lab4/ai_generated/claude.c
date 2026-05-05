#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

// Function prototypes
void parse_command(char *line, char **args);
int execute_command(char **args);
int builtin_cd(char **args);
int builtin_exit(char **args);
int builtin_help(char **args);

// Built-in command structure
struct {
    char *name;
    int (*func)(char **);
} builtins[] = {
    {"cd", builtin_cd},
    {"exit", builtin_exit},
    {"help", builtin_help},
    {NULL, NULL}
};

int main(void) {
    char line[MAX_LINE];
    char *args[MAX_ARGS];
    
    while (1) {
        // Print prompt
        printf("myshell> ");
        fflush(stdout);
        
        // Read command line
        if (fgets(line, MAX_LINE, stdin) == NULL) {
            printf("\n");
            break;
        }
        
        // Remove newline character
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Parse command into arguments
        parse_command(line, args);
        
        // Execute command if args[0] is not NULL
        if (args[0] != NULL) {
            execute_command(args);
        }
    }
    
    return 0;
}

void parse_command(char *line, char **args) {
    int i = 0;
    char *token = strtok(line, " \t");
    
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
}

int execute_command(char **args) {
    // Check for built-in commands
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(args[0], builtins[i].name) == 0) {
            return builtins[i].func(args);
        }
    }
    
    // Execute external command
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // Child process
        if (execvp(args[0], args) < 0) {
            perror(args[0]);
            exit(1);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
    
    return 0;
}

int builtin_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "cd: expected argument\n");
        return 1;
    }
    
    if (chdir(args[1]) != 0) {
        perror("cd");
        return 1;
    }
    
    return 0;
}

int builtin_exit(char **args) {
    exit(0);
}

int builtin_help(char **args) {
    printf("Simple Unix Shell\n");
    printf("Built-in commands:\n");
    printf("  cd <directory>  - Change directory\n");
    printf("  help            - Display this help message\n");
    printf("  exit            - Exit the shell\n");
    printf("\nAll other commands are executed as external programs.\n");
    return 0;
}