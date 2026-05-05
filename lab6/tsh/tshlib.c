#include "tshlib.h"

int connectTshLib(u_short port)
{
    short tsh_port;
    u_long tsh_host;
    int sock;

    tsh_host = inet_addr("127.0.0.1");
    tsh_port = htons(port);

    if ((sock = get_socket()) == -1)
        return -1;

    if (!do_connect(sock, tsh_host, tsh_port))
    {
        close(sock);
        return -1;
    }

    return sock;
}

int tsh_shell_cmd(u_short port, const char *command, char *output, int out_size)
{
    int tshsock;
    u_short this_op;
    tsh_shell_it out;
    tsh_shell_ot in;
    int len;

    memset(&out, 0, sizeof(out));
    memset(&in, 0, sizeof(in));

    if (command == NULL || output == NULL || out_size <= 0)
        return 0;

    tshsock = connectTshLib(port);
    if (tshsock == -1)
        return 0;

    this_op = htons(TSH_OP_SHELL);
    if (!writen(tshsock, (char *)&this_op, sizeof(this_op)))
    {
        close(tshsock);
        return 0;
    }

    strncpy(out.command, command, sizeof(out.command) - 1);
    out.command[sizeof(out.command) - 1] = '\0';

    if (!writen(tshsock, (char *)&out, sizeof(out)))
    {
        close(tshsock);
        return 0;
    }

    if (!readn(tshsock, (char *)&in, sizeof(in)))
    {
        close(tshsock);
        return 0;
    }

    if (ntohs(in.status) != SUCCESS)
    {
        close(tshsock);
        return 0;
    }

    len = ntohl(in.length);
    if (len <= 0)
    {
        output[0] = '\0';
        close(tshsock);
        return 1;
    }

    if (len > out_size)
        len = out_size;

    if (!readn(tshsock, output, len))
    {
        close(tshsock);
        return 0;
    }

    output[len - 1] = '\0';
    close(tshsock);
    return 1;
}