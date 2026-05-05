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

int main(){
    int arr[5000];
    clock_t start, end;
    double elapsed;
    srand(time(NULL));

    for(int i = 0; i < 5000; i++){
        arr[i] = rand();
    }

    start = clock();
    bubbleSort(arr, 5000);
    end = clock();

    elapsed = (double)(end - start)/CLOCKS_PER_SEC;
    printf("Elapsed time: %f seconds", elapsed);

    return 0;
}