#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int **a, **b, **c;
    int n, row_start, row_end;
} Task;

void* worker(void* arg) {
    Task* t = (Task*)arg;
    int n = t->n;
    for (int i = t->row_start; i < t->row_end; i++) {
        for (int j = 0; j < n; j++) {
            int s = 0;
            for (int k = 0; k < n; k++)
                s += t->a[i][k] * t->b[k][j];
            t->c[i][j] = s;
        }
    }
    return NULL;
}

static int** alloc_matrix(int n) {
    int **m = malloc(n * sizeof(*m));
    m[0] = malloc(n * n * sizeof(**m));
    for (int i = 1; i < n; i++) m[i] = m[0] + i * n;
    return m;
}

int main() {
    int n;
    scanf("%d", &n);

    int **a = alloc_matrix(n);
    int **b = alloc_matrix(n);
    int **c = alloc_matrix(n);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) scanf("%d", &a[i][j]);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) scanf("%d", &b[i][j]);

    int num_threads = 8;
    pthread_t th[num_threads];
    Task tasks[num_threads];

    int rows_per_thread = (n + num_threads - 1) / num_threads;
    for (int t = 0; t < num_threads; t++) {
        tasks[t].a = a; tasks[t].b = b; tasks[t].c = c; tasks[t].n = n;
        tasks[t].row_start = t * rows_per_thread;
        tasks[t].row_end = (t + 1) * rows_per_thread;
        if (tasks[t].row_end > n) tasks[t].row_end = n;
        pthread_create(&th[t], NULL, worker, &tasks[t]);
    }
    for (int t = 0; t < num_threads; t++) pthread_join(th[t], NULL);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) printf("%d ", c[i][j]);
        printf("\n");
    }

    free(a[0]); free(a);
    free(b[0]); free(b);
    free(c[0]); free(c);
    return 0;
}