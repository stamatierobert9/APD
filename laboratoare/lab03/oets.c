#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "pthread_barrier_mac.h"

int N;
int P;
int *v;
int *vQSort;

pthread_barrier_t barrier; // barrier pentru sincronizare intre faze

void compare_vectors(int *a, int *b) {
    int i;
    for (i = 0; i < N; i++) {
        if (a[i] != b[i]) {
            printf("Sortare incorecta\n");
            return;
        }
    }
    printf("Sortare corecta\n");
}

void display_vector(int *v) {
    int i;
    int display_width = 2 + (int)log10(N);
    for (i = 0; i < N; i++) {
        printf("%*i", display_width, v[i]);
    }
    printf("\n");
}

int cmp(const void *a, const void *b) {
    int A = *(const int*)a;
    int B = *(const int*)b;
    return A - B;
}

void get_args(int argc, char **argv)
{
    if (argc < 3) {
        printf("Numar insuficient de parametri: ./oets N P\n");
        exit(1);
    }
    N = atoi(argv[1]);
    P = atoi(argv[2]);
    if (N <= 0 || P <= 0) {
        printf("Parametri invalizi: N si P trebuie sa fie pozitivi.\n");
        exit(1);
    }
}

void init()
{
    int i;
    v = (int*)malloc(sizeof(int) * N);
    vQSort = (int*)malloc(sizeof(int) * N);

    if (v == NULL || vQSort == NULL) {
        printf("Eroare la malloc!");
        exit(1);
    }

    srand(42);
    for (i = 0; i < N; i++)
        v[i] = rand() % N;
}

void print()
{
    printf("v:\n");
    display_vector(v);
    printf("vQSort:\n");
    display_vector(vQSort);
    compare_vectors(v, vQSort);
}

void *thread_function(void *arg)
{
    int thread_id = *(int *)arg;

    // Rulam N faze OETS (suficient pentru a sorta complet)
    for (int phase = 0; phase < N; phase++) {
        // phase pară: comparam (0,1), (2,3), ...
        // phase impară: comparam (1,2), (3,4), ...
        int start = (phase & 1) ? (1 + 2 * thread_id) : (0 + 2 * thread_id);

        for (int i = start; i < N - 1; i += 2 * P) {
            if (v[i] > v[i + 1]) {
                int aux = v[i];
                v[i] = v[i + 1];
                v[i + 1] = aux;
            }
        }

        // Sincronizam toate thread-urile inainte de faza urmatoare
        pthread_barrier_wait(&barrier);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    get_args(argc, argv);
    init();

    int i;
    pthread_t *tid = (pthread_t*)malloc(sizeof(pthread_t) * P);
    int *thread_id = (int*)malloc(sizeof(int) * P);

    // vectorul etalon
    for (i = 0; i < N; i++)
        vQSort[i] = v[i];
    qsort(vQSort, N, sizeof(int), cmp);

    // initializam barrier-ul pentru P thread-uri
    if (pthread_barrier_init(&barrier, NULL, P) != 0) {
        printf("Eroare la initializarea barrier-ului\n");
        exit(1);
    }

    // cream thread-urile
    for (i = 0; i < P; i++) {
        thread_id[i] = i;
        if (pthread_create(&tid[i], NULL, thread_function, &thread_id[i]) != 0) {
            printf("Eroare la crearea thread-ului %d\n", i);
            exit(1);
        }
    }

    // asteptam thread-urile
    for (i = 0; i < P; i++) {
        pthread_join(tid[i], NULL);
    }

    // distrugem barrier-ul
    pthread_barrier_destroy(&barrier);

    // afisam si validam
    print();

    free(v);
    free(vQSort);
    free(tid);
    free(thread_id);

    return 0;
}
