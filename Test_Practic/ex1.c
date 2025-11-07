#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

typedef struct {
    int tid;
    int N;
    int P;
    int64_t *a;
    int64_t *b;
    int64_t *partial;
} thread_arg_t;

void *worker(void *argptr) {
    thread_arg_t *arg = (thread_arg_t *)argptr;
    int tid = arg->tid;
    int N = arg->N;
    int P = arg->P;
    int64_t *a = arg->a;
    int64_t *b = arg->b;
    int64_t *partial = arg->partial;
    size_t plane = (size_t)N * N;
    int64_t *myplane = partial + (size_t)tid * plane;

    for (int i = 0; i < N; ++i) {
        size_t rowbase_a = (size_t)i * N;
        size_t rowbase_out = (size_t)i * N;
        for (int j = 0; j < N; ++j) {
            int64_t sum = 0;
            for (int k = tid; k < N; k += P) {
                sum += a[rowbase_a + k] * b[(size_t)k * N + j];
            }
            myplane[rowbase_out + j] = sum;
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s N P\n", argv[0]);
        return 1;
    }
    int N = atoi(argv[1]);
    int P = atoi(argv[2]);
    if (N <= 0 || P <= 0) {
        fprintf(stderr, "N and P must be positive integers\n");
        return 1;
    }
    if (N > 1500) {
        fprintf(stderr, "N too large (practical limit 1500)\n");
        return 1;
    }

    size_t nn = (size_t)N * N;
    int64_t *a = malloc(nn * sizeof(int64_t));
    int64_t *b = malloc(nn * sizeof(int64_t));
    int64_t *c = calloc(nn, sizeof(int64_t));
    if (!a || !b || !c) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }

    for (int i = 0; i < N; ++i) {
        size_t row = (size_t)i * N;
        for (int j = 0; j < N; ++j) {
            a[row + j] = 7 + (i + j) % 100;
            b[row + j] = 50 + (i + j) % 100;
        }
    }

    int64_t *partial = malloc((size_t)P * nn * sizeof(int64_t));
    if (!partial) {
        fprintf(stderr, "Allocation of partial arrays failed\n");
        free(a); free(b); free(c);
        return 1;
    }

    pthread_t *threads = malloc((size_t)P * sizeof(pthread_t));
    thread_arg_t *targs = malloc((size_t)P * sizeof(thread_arg_t));
    if (!threads || !targs) {
        fprintf(stderr, "Allocation failed\n");
        free(a); free(b); free(c); free(partial);
        return 1;
    }

    for (int t = 0; t < P; ++t) {
        targs[t].tid = t;
        targs[t].N = N;
        targs[t].P = P;
        targs[t].a = a;
        targs[t].b = b;
        targs[t].partial = partial;
        if (pthread_create(&threads[t], NULL, worker, &targs[t]) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", t);
            for (int x = 0; x < t; ++x) pthread_join(threads[x], NULL);
            free(a); free(b); free(c); free(partial); free(threads); free(targs);
            return 1;
        }
    }

    for (int t = 0; t < P; ++t) pthread_join(threads[t], NULL);

    for (int t = 0; t < P; ++t) {
        size_t base = (size_t)t * nn;
        for (size_t idx = 0; idx < nn; ++idx) {
            c[idx] += partial[base + idx];
        }
    }

    for (int i = 0; i < N; ++i) {
        size_t row = (size_t)i * N;
        for (int j = 0; j < N; ++j) {
            printf("%lld", (long long)c[row + j]);
            if (j + 1 < N) putchar(' ');
        }
        putchar('\n');
    }

    free(a); free(b); free(c); free(partial); free(threads); free(targs);
    return 0;
}