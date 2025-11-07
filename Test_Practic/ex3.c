#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MOD 1337

enum { ROLE_MIN = 0, ROLE_MAX = 1, ROLE_SUM = 2, ROLE_PROD = 3 };

static int *v;
static int N;

/* Sincronizare doar pentru ordinea afișării */
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cv  = PTHREAD_COND_INITIALIZER;
static int turn = 0; /* 0=min, 1=max, 2=sum, 3=prod */

typedef struct {
    int role; /* ce calculează acest thread */
} Task;

static void wait_and_print_turn(int my_role, const char *label, const char *fmt, long long value)
{
    pthread_mutex_lock(&mtx);
    while (turn != my_role)
        pthread_cond_wait(&cv, &mtx);

    /* Afișează exact în ordinea cerută */
    printf("%s = ", label);
    printf(fmt, value);
    printf("\n");
    fflush(stdout);

    turn++;
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&mtx);
}

static void* worker(void *arg)
{
    Task *t = (Task*)arg;
    int role = t->role;

    if (role == ROLE_MIN) {
        int mn = v[0];
        for (int i = 1; i < N; i++)
            if (v[i] < mn) mn = v[i];
        wait_and_print_turn(ROLE_MIN, "minim", "%d", (long long)mn);
    } else if (role == ROLE_MAX) {
        int mx = v[0];
        for (int i = 1; i < N; i++)
            if (v[i] > mx) mx = v[i];
        wait_and_print_turn(ROLE_MAX, "maxim", "%d", (long long)mx);
    } else if (role == ROLE_SUM) {
        long long s = 0;
        for (int i = 0; i < N; i++)
            s += v[i];
        wait_and_print_turn(ROLE_SUM, "suma", "%lld", s);
    } else { /* ROLE_PROD */
        long long p = 1;
        for (int i = 0; i < N; i++) {
            p = (p * v[i]) % MOD;
        }
        wait_and_print_turn(ROLE_PROD, "produs", "%lld", p);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        /* main NU afișează rezultatele; mesaj de utilizare e ok */
        fprintf(stderr, "Utilizare: %s N\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]);
    if (N <= 0) {
        fprintf(stderr, "N trebuie sa fie > 0\n");
        return 1;
    }

    v = (int*)malloc((size_t)N * sizeof(int));
    if (!v) {
        perror("malloc");
        return 1;
    }

    /* Inițializare vector cu valori pseudo-aleatoare deterministe */
    srand(12345);
    for (int i = 0; i < N; i++)
        v[i] = rand() % 100;

    pthread_t th[4];
    Task tasks[4];

    /* Creează exact 4 thread-uri, fiecare cu rolul lui */
    for (int r = 0; r < 4; r++) {
        tasks[r].role = r;
        if (pthread_create(&th[r], NULL, worker, &tasks[r]) != 0) {
            perror("pthread_create");
            free(v);
            return 1;
        }
    }

    for (int r = 0; r < 4; r++)
        pthread_join(th[r], NULL);

    free(v);
    return 0;
}
