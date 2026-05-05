#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tshlib.h"

static int failures = 0;

static void expect(int condition, const char *name)
{
    printf("%s: %s\n", condition ? "PASS" : "FAIL", name);
    if (!condition)
        failures++;
}

int main(int argc, char **argv)
{
    u_short port;
    char tuple_name[TUPLENAME_LEN];
    char tpname[TUPLENAME_LEN];
    char output[256];
    void *tuple = NULL;
    int length = 0;
    int priority = 0;

    if (argc < 2) {
        printf("Usage: %s port\n", argv[0]);
        return 1;
    }

    port = (u_short)atoi(argv[1]);
    snprintf(tuple_name, sizeof(tuple_name), "API_%ld", (long)getpid());

    expect(tsh_put(port, tuple_name, "world", 6, 1), "tsh_put");

    if (tsh_read(port, tuple_name, tpname, &tuple, &length, &priority)) {
        expect(strcmp(tpname, tuple_name) == 0 &&
               strcmp((char *)tuple, "world") == 0 &&
               length == 6 &&
               priority == 1,
               "tsh_read expected tuple");
        free(tuple);
        tuple = NULL;
    } else {
        expect(0, "tsh_read expected tuple");
    }

    if (tsh_get(port, tuple_name, tpname, &tuple, &length, &priority)) {
        expect(strcmp(tpname, tuple_name) == 0 &&
               strcmp((char *)tuple, "world") == 0,
               "tsh_get expected tuple");
        free(tuple);
        tuple = NULL;
    } else {
        expect(0, "tsh_get expected tuple");
    }

    memset(output, 0, sizeof(output));
    expect(tsh_shell_cmd(port, "printf api-shell", output, sizeof(output)),
           "tsh_shell_cmd");
    expect(strcmp(output, "api-shell") == 0,
           "tsh_shell_cmd expected output");

    printf("Summary: %d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
