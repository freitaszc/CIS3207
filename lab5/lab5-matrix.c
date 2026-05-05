#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define N 1000

void initialize_matrices(int A[N][N], int B[N][N], int C[N][N]){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            A[i][j] = (i + j) % 10;
            B[i][j] = (i * j) % 10;
            C[i][j] = 0;
        }
    }
}

void ijk(int A[N][N], int B[N][N], int C[N][N]){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            for(int k = 0; k < N; k++){
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void ikj(int A[N][N], int B[N][N], int C[N][N]){
    for(int i = 0; i < N; i++){
        for(int k = 0; k < N; k++){
            for(int j = 0; j < N; j++){
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void jik(int A[N][N], int B[N][N], int C[N][N]){
    for(int j = 0; j < N; j++){
        for(int i = 0; i < N; i++){
            for(int k = 0; k < N; k++){
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void jki(int A[N][N], int B[N][N], int C[N][N]){
    for(int j = 0; j < N; j++){
        for(int k = 0; k < N; k++){
            for(int i = 0; i < N; i++){
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void kij(int A[N][N], int B[N][N], int C[N][N]){
    for(int k = 0; k < N; k++){
        for(int i = 0; i < N; i++){
            for(int j = 0; j < N; j++){
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void kji(int A[N][N], int B[N][N], int C[N][N]){
    for(int k = 0; k < N; k++){
        for(int j = 0; j < N; j++){
            for(int i = 0; i < N; i++){
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void save_file(const char *filename, int C[N][N], const char *order, double elapsed){
    FILE *fp = fopen(filename, "w");
    if(fp == NULL){
        perror("Error opening output file");
        exit(1);
    }

    fprintf(fp, "Loop order: %s\n", order);
    fprintf(fp, "Elapsed time: %.6f seconds\n", elapsed);
    fprintf(fp, "Result matrix C:\n");

    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            fprintf(fp, "%d ", C[i][j]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

int main(int argc, char *argv[]){
    static int A[N][N];
    static int B[N][N];
    static int C[N][N];

    char output_file[50];
    struct timeval start, end;
    double elapsed;

    if(argc != 2){
        printf("Usage: %s [ijk|ikj|jik|jki|kij|kji]\n", argv[0]);
        return 1;
    }

    initialize_matrices(A, B, C);

    gettimeofday(&start, NULL);

    if(strcmp(argv[1], "ijk") == 0){
        ijk(A, B, C);
        strcpy(output_file, "result_ijk.txt");
    } else if(strcmp(argv[1], "ikj") == 0){
        ikj(A, B, C);
        strcpy(output_file, "result_ikj.txt");
    } else if(strcmp(argv[1], "jik") == 0){
        jik(A, B, C);
        strcpy(output_file, "result_jik.txt");
    } else if(strcmp(argv[1], "jki") == 0){
        jki(A, B, C);
        strcpy(output_file, "result_jki.txt");
    } else if(strcmp(argv[1], "kij") == 0){
        kij(A, B, C);
        strcpy(output_file, "result_kij.txt");
    } else if(strcmp(argv[1], "kji") == 0){
        kji(A, B, C);
        strcpy(output_file, "result_kji.txt");
    } else {
        printf("Invalid loop order.\n");
        printf("Use one of: ijk, ikj, jik, jki, kij, kji\n");
        return 1;
    }

    gettimeofday(&end, NULL);

    elapsed = (end.tv_sec - start.tv_sec) +
              (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Loop order: %s\n", argv[1]);
    printf("Elapsed time: %.6f seconds\n", elapsed);

    save_file(output_file, C, argv[1], elapsed);
    printf("Result saved to %s\n", output_file);

    return 0;
}