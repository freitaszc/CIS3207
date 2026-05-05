#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include "tshlib.h"

#define MAX_N 64
#define RESULT_TIMEOUT_SECONDS 2

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

static volatile sig_atomic_t timeout_flag = 0;

void alarm_handler(int sig)
{
    (void)sig;
    timeout_flag = 1;
}

void print_matrix(int *m, int n)
{
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%4d ", m[i * n + j]);
        }
        printf("\n");
    }
}

double elapsed_seconds(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) +
           (end.tv_usec - start.tv_usec) / 1000000.0;
}

int main(int argc, char *argv[])
{
    u_short port;
    int n, workers, granularity;
    matrix_msg A, B;
    int C[MAX_N * MAX_N];
    int i, r, c;
    int total_tasks = 0;
    int completed_tasks = 0;
    int run_id;
    task_msg tasks[MAX_N];
    int completed[MAX_N];
    int attempts[MAX_N];
    time_t last_sent[MAX_N];
    char expr[64];
    void *tuple = NULL;
    int len, pri;
    char tpname[TUPLENAME_LEN];
    struct timeval start, end;

    if (argc < 6) {
        printf("Usage: %s <port> <matrix_size> <workers> <granularity> <run_id>\n", argv[0]);
        return 1;
    }

    port = atoi(argv[1]);
    n = atoi(argv[2]);
    workers = atoi(argv[3]);
    granularity = atoi(argv[4]);
    run_id = atoi(argv[5]);

    if (n <= 0 || n > MAX_N) {
        printf("matrix_size must be between 1 and %d\n", MAX_N);
        return 1;
    }

    if (workers <= 0) {
        printf("workers must be > 0\n");
        return 1;
    }

    if (granularity <= 0 || granularity > n) {
        printf("granularity must be between 1 and %d\n", n);
        return 1;
    }

    if (run_id <= 0) {
        printf("run_id must be > 0\n");
        return 1;
    }

    memset(&A, 0, sizeof(A));
    memset(&B, 0, sizeof(B));
    memset(C, 0, sizeof(C));
    memset(tasks, 0, sizeof(tasks));
    memset(completed, 0, sizeof(completed));
    memset(attempts, 0, sizeof(attempts));
    memset(last_sent, 0, sizeof(last_sent));

    A.n = n;
    B.n = n;

    for (i = 0; i < n * n; i++) {
        A.data[i] = i + 1;
        B.data[i] = 1;
    }

    sprintf(expr, "A_%d", run_id);
    if (!tsh_put(port, expr, &A, sizeof(A), 1)) {
        printf("Failed to put A\n");
        return 1;
    }

    sprintf(expr, "B_%d", run_id);
    if (!tsh_put(port, expr, &B, sizeof(B), 1)) {
        printf("Failed to put B\n");
        return 1;
    }

    for (i = 0; i < n; i += granularity) {
        task_msg task;
        char name[32];

        memset(&task, 0, sizeof(task));
        task.run_id = run_id;
        task.task_id = total_tasks;
        task.start_row = i;
        task.end_row = i + granularity;
        if (task.end_row > n)
            task.end_row = n;
        task.n = n;

        sprintf(name, "TASK_%d_%d", run_id, total_tasks);

        if (!tsh_put(port, name, &task, sizeof(task), 1)) {
            printf("Failed to put %s\n", name);
            return 1;
        }

        tasks[total_tasks] = task;
        attempts[total_tasks] = 1;
        last_sent[total_tasks] = time(NULL);
        total_tasks++;
    }

    signal(SIGALRM, alarm_handler);
    alarm(1);

    gettimeofday(&start, NULL);

    while (completed_tasks < total_tasks) {
        for (i = 0; i < total_tasks; i++) {
            result_msg *res;

            sprintf(expr, "^RESULT_%d_%d$", run_id, i);

            if (!tsh_get(port, expr, tpname, &tuple, &len, &pri))
                continue;

            if (len != sizeof(result_msg)) {
                printf("Ignoring malformed result tuple %s\n", tpname);
                free(tuple);
                tuple = NULL;
                continue;
            }

            res = (result_msg *)tuple;
            if (res->run_id != run_id || res->task_id < 0 ||
                res->task_id >= total_tasks || res->task_id != i) {
                printf("Ignoring unexpected result tuple %s for run %d task %d\n",
                       tpname, res->run_id, res->task_id);
                free(tuple);
                tuple = NULL;
                continue;
            }

            if (completed[res->task_id]) {
                printf("Ignoring duplicate RESULT_%d\n", res->task_id);
                free(tuple);
                tuple = NULL;
                continue;
            }

            for (r = res->start_row; r < res->end_row; r++) {
                for (c = 0; c < n; c++) {
                    C[r * n + c] = res->data[r * n + c];
                }
            }

            completed[res->task_id] = 1;
            completed_tasks++;

            free(tuple);
            tuple = NULL;
        }

        if (timeout_flag) {
            time_t now;

            timeout_flag = 0;
            now = time(NULL);

            for (i = 0; i < total_tasks; i++) {
                char name[32];

                if (completed[i])
                    continue;

                if (now - last_sent[i] < RESULT_TIMEOUT_SECONDS)
                    continue;

                sprintf(name, "TASK_%d_%d", run_id, i);
                if (tsh_put(port, name, &tasks[i], sizeof(tasks[i]), 1)) {
                    attempts[i]++;
                    last_sent[i] = now;
                    printf("Timeout: reissued %s attempt %d\n", name, attempts[i]);
                }
            }

            alarm(1);
        }

        usleep(100000);
    }

    alarm(0);
    gettimeofday(&end, NULL);

    for (i = 0; i < workers; i++) {
        int stop_value = 1;
        char stop_name[32];

        sprintf(stop_name, "STOP_%d_%d", run_id, i);
        tsh_put(port, stop_name, &stop_value, sizeof(stop_value), 1);
    }

    printf("Matrix size: %d\n", n);
    printf("Workers: %d\n", workers);
    printf("Granularity: %d\n", granularity);
    printf("Total tasks: %d\n", total_tasks);
    printf("Elapsed time: %.6f seconds\n", elapsed_seconds(start, end));

    if (n <= 8) {
        printf("\nResult Matrix C:\n");
        print_matrix(C, n);
    }

    return 0;
}
