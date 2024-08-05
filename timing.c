#include <stdio.h>
#include <time.h>

int main() {
    const long long int iterations = 6000;
    clock_t start = clock();
    for (long long int i = 0; i < iterations; ++i) {
    }
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Time taken: %f seconds\n", time_taken);
    return 0;
}