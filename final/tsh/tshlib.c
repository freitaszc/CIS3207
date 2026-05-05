#include "tshlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int connectTshLib(u_short port)
{
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return -1;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}

int tsh_put(u_short port, const char *tpname, const void *tuple, int length, int priority)
{
    int sock;
    u_short op;
    tsh_put_it out;
    tsh_put_ot in;

    if (!tpname || !tuple || length <= 0)
        return 0;

    sock = connectTshLib(port);
    if (sock < 0)
        return 0;

    op = htons(TSH_OP_PUT);
    if (!writen(sock, (char *)&op, sizeof(op))) {
        close(sock);
        return 0;
    }

    memset(&out, 0, sizeof(out));
    strncpy(out.name, tpname, TUPLENAME_LEN - 1);
    out.name[TUPLENAME_LEN - 1] = '\0';
    out.priority = htons(priority);
    out.host = inet_addr("127.0.0.1");
    out.port = 0;
    out.length = htonl(length);
    out.proc_id = htonl(getpid());

    if (!writen(sock, (char *)&out, sizeof(out))) {
        close(sock);
        return 0;
    }

    if (!writen(sock, (char *)tuple, length)) {
        close(sock);
        return 0;
    }

    if (!readn(sock, (char *)&in, sizeof(in))) {
        close(sock);
        return 0;
    }

    close(sock);
    return (ntohs(in.status) == SUCCESS);
}

static int tsh_fetch(u_short port, u_short op_code, const char *expr,
                     char *tpname, void **tuple,
                     int *length, int *priority)
{
    int sock;
    u_short op;
    tsh_get_it out;
    tsh_get_ot1 in1;
    tsh_get_ot2 in2;
    int len;
    int pri;
    void *buf;

    if (!expr || !tpname || !tuple || !length || !priority)
        return 0;

    sock = connectTshLib(port);
    if (sock < 0)
        return 0;

    op = htons(op_code);
    if (!writen(sock, (char *)&op, sizeof(op))) {
        close(sock);
        return 0;
    }

    memset(&out, 0, sizeof(out));
    strncpy(out.expr, expr, TUPLENAME_LEN - 1);
    out.expr[TUPLENAME_LEN - 1] = '\0';
    out.host = inet_addr("127.0.0.1");
    out.port = 0;
    out.len = htonl((unsigned int)-1);
    out.proc_id = htonl(getpid());
    out.cidport = 0;

    if (!writen(sock, (char *)&out, sizeof(out))) {
        close(sock);
        return 0;
    }

    if (!readn(sock, (char *)&in1, sizeof(in1))) {
        close(sock);
        return 0;
    }

    if (ntohs(in1.status) != SUCCESS) {
        close(sock);
        return 0;
    }

    if (!readn(sock, (char *)&in2, sizeof(in2))) {
        close(sock);
        return 0;
    }

    len = ntohl(in2.length);
    pri = ntohs(in2.priority);

    buf = malloc(len);
    if (!buf) {
        close(sock);
        return 0;
    }

    if (!readn(sock, buf, len)) {
        free(buf);
        close(sock);
        return 0;
    }

    strncpy(tpname, in2.name, TUPLENAME_LEN - 1);
    tpname[TUPLENAME_LEN - 1] = '\0';

    *tuple = buf;
    *length = len;
    *priority = pri;

    close(sock);
    return 1;
}

int tsh_get(u_short port, const char *expr,
            char *tpname,
            void **tuple,
            int *length,
            int *priority)
{
    return tsh_fetch(port, TSH_OP_GET, expr, tpname, tuple, length, priority);
}

int tsh_read(u_short port, const char *expr,
             char *tpname,
             void **tuple,
             int *length,
             int *priority)
{
    return tsh_fetch(port, TSH_OP_READ, expr, tpname, tuple, length, priority);
}

int tsh_exit_cmd(u_short port)
{
    int sock;
    u_short op;
    tsh_exit_ot in;

    sock = connectTshLib(port);
    if (sock < 0)
        return 0;

    op = htons(TSH_OP_EXIT);
    if (!writen(sock, (char *)&op, sizeof(op))) {
        close(sock);
        return 0;
    }

    if (!readn(sock, (char *)&in, sizeof(in))) {
        close(sock);
        return 0;
    }

    close(sock);
    return (ntohs(in.status) == SUCCESS);
}

int tsh_shell_cmd(u_short port, const char *command, char *output, int out_size)
{
    int sock;
    u_short op;
    tsh_shell_it out;
    tsh_shell_ot in;
    int len;

    if (!command || !output || out_size <= 0)
        return 0;

    sock = connectTshLib(port);
    if (sock < 0)
        return 0;

    op = htons(TSH_OP_SHELL);
    if (!writen(sock, (char *)&op, sizeof(op))) {
        close(sock);
        return 0;
    }

    memset(&out, 0, sizeof(out));
    strncpy(out.command, command, TUPLENAME_LEN - 1);
    out.command[TUPLENAME_LEN - 1] = '\0';

    if (!writen(sock, (char *)&out, sizeof(out))) {
        close(sock);
        return 0;
    }

    if (!readn(sock, (char *)&in, sizeof(in))) {
        close(sock);
        return 0;
    }

    len = ntohl(in.length);
    if (len >= out_size)
        len = out_size - 1;

    if (!readn(sock, output, len)) {
        close(sock);
        return 0;
    }

    output[len] = '\0';

    close(sock);
    return (ntohs(in.status) == SUCCESS);
}
