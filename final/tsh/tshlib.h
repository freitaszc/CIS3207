#ifndef TSHLIB_H
#define TSHLIB_H

#include "synergy.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int connectTshLib(u_short port);

int tsh_put(u_short port, const char *tpname, const void *tuple, int length, int priority);
int tsh_get(u_short port, const char *expr, char *tpname, void **tuple, int *length, int *priority);
int tsh_read(u_short port, const char *expr, char *tpname, void **tuple, int *length, int *priority);
int tsh_exit_cmd(u_short port);
int tsh_shell_cmd(u_short port, const char *command, char *output, int out_size);

#endif