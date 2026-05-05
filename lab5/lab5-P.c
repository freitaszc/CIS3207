#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void bubbleSort(int *a, int n){
    if (n < 2)
        return;
    for (int i = 0; i < n - 1; i++)
    {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++)
        {
            if (a[j] > a[j + 1])
            {
                int tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped)
            break;
    }
}

static void merge(int *a, int left, int mid, int right, int *temp) {
    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (a[i] <= a[j]) {
            temp[k] = a[i];
            i++;
        } else {
            temp[k] = a[j];
            j++;
        }
        k++;
    }

    while (i <= mid) {
        temp[k] = a[i];
        i++;
        k++;
    }

    while (j <= right) {
        temp[k] = a[j];
        j++;
        k++;
    }

    for (int x = left; x <= right; x++) {
        a[x] = temp[x];
    }
}

int main(){
    int arr[5000];
    int temp[5000];
    clock_t start, end;
    double elapsed;
    srand(12345);

    for(int i = 0; i < 5000; i++){
        arr[i] = rand();
    }

    int partSize = 2500;

    start = clock();

    bubbleSort(arr, partSize);
    bubbleSort(arr + partSize, partSize);

    merge(arr, 0, 2499, 4999, temp);

    end = clock();

    elapsed = (double)(end - start)/CLOCKS_PER_SEC;

    FILE *fp = fopen("lab5-P-output.txt", "w");
    if (!fp)
    {
        perror("fopen");
        return 1;
    }

    for (int i = 0; i < 5000; i++)
    {
        fprintf(fp, "%d\n", arr[i]);
    }

    fprintf(fp, "Elapsed time: %f seconds\n", elapsed);
    fclose(fp);
    
    printf("Elapsed time: %f seconds\n", elapsed);

    return 0;
}