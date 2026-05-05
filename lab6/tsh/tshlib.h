#ifndef TSHLIB_H
#define TSHLIB_H

#include "synergy.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int connectTshLib(u_short port);
int tsh_shell_cmd(u_short port, const char *command, char *output, int out_size);

#endif