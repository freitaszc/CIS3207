#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tshlib.h"

#define MAX_N 64

typedef struct {
    int n;
    int data[MAX_N * MAX_N];
} matrix_msg;

typedef struct {
    int run_id;
    int task_id;
    int start_row;
    int end_row;
    int n;
} task_msg;

typedef struct {
    int run_id;
    int task_id;
    int start_row;
    int end_row;
    int n;
    int data[MAX_N * MAX_N];
} result_msg;

int main(int argc, char *argv[])
{
    u_short port;
    int run_id;
    char expr[64];
    char tpname[TUPLENAME_LEN];
    void *tupleA = NULL;
    void *tupleB = NULL;
    void *tupleTask = NULL;
    int len, pri;
    matrix_msg *A;
    matrix_msg *B;

    if (argc < 3) {
        printf("Usage: %s <port> <run_id>\n", argv[0]);
        return 1;
    }

    port = atoi(argv[1]);
    run_id = atoi(argv[2]);
    if (run_id <= 0) {
        printf("run_id must be > 0\n");
        return 1;
    }

    printf("Worker waiting for data...\n");

    sprintf(expr, "^A_%d$", run_id);
    while (!tsh_read(port, expr, tpname, &tupleA, &len, &pri)) {
        usleep(100000);
    }
    A = (matrix_msg *)tupleA;

    sprintf(expr, "^B_%d$", run_id);
    while (!tsh_read(port, expr, tpname, &tupleB, &len, &pri)) {
        usleep(100000);
    }
    B = (matrix_msg *)tupleB;

    while (1) {
        task_msg *task;
        result_msg result;
        char result_name[32];
        int n, i, j, k;

        tupleTask = NULL;

        sprintf(expr, "STOP_%d_", run_id);
        if (tsh_get(port, expr, tpname, &tupleTask, &len, &pri)) {
            free(tupleTask);
            break;
        }

        sprintf(expr, "TASK_%d_", run_id);
        if (!tsh_get(port, expr, tpname, &tupleTask, &len, &pri)) {
            usleep(100000);
            continue;
        }

        task = (task_msg *)tupleTask;
        if (task->run_id != run_id) {
            free(tupleTask);
            continue;
        }
        printf("Worker received task %d rows %d to %d\n",
               task->task_id, task->start_row, task->end_row - 1);

        if (getenv("WORKER_DELAY_SECONDS")) {
            sleep(atoi(getenv("WORKER_DELAY_SECONDS")));
        }

        memset(&result, 0, sizeof(result));
        result.run_id = task->run_id;
        result.task_id = task->task_id;
        result.start_row = task->start_row;
        result.end_row = task->end_row;
        result.n = task->n;
        n = task->n;

        for (i = task->start_row; i < task->end_row; i++) {
            for (j = 0; j < n; j++) {
                for (k = 0; k < n; k++) {
                    result.data[i * n + j] += A->data[i * n + k] * B->data[k * n + j];
                }
            }
        }

        sprintf(result_name, "RESULT_%d_%d", task->run_id, task->task_id);
        tsh_put(port, result_name, &result, sizeof(result), 1);

        free(tupleTask);
    }

    printf("Worker exiting\n");

    free(tupleA);
    free(tupleB);
    return 0;
}
