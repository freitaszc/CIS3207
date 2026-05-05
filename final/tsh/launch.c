#include "launch.h"

int main(int argc, char **argv)
{
    char output[2048];
    char command[256];
    int i;

    if (argc < 3)
    {
        printf("Usage: %s port command\n", argv[0]);
        printf("Example: %s 2110 ls\n", argv[0]);
        return 1;
    }

    command[0] = '\0';

    for (i = 2; i < argc; i++)
    {
        strcat(command, argv[i]);
        if (i < argc - 1)
            strcat(command, " ");
    }

    memset(output, 0, sizeof(output));

    if (!tsh_shell_cmd(atoi(argv[1]), command, output, sizeof(output)))
    {
        printf("OP_SHELL failed\n");
        return 1;
    }

    printf("%s", output);
    return 0;
}