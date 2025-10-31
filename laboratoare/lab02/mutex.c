#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 2

int a = 0;

// Declarați mutexul global
pthread_mutex_t lock;

// Funcția rulată de fiecare thread
void *f(void *arg)
{
    // Blocăm mutexul pentru a proteja secțiunea critică
    pthread_mutex_lock(&lock);

    a += 2; // secțiune critică — modificăm variabila partajată

    // Deblocăm mutexul după ce am terminat
    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int i, r;
    void *status;
    pthread_t threads[NUM_THREADS];
    int arguments[NUM_THREADS];

    // Inițializăm mutexul
    pthread_mutex_init(&lock, NULL);

    // Creăm thread-urile
    for (i = 0; i < NUM_THREADS; i++) {
        arguments[i] = i;
        r = pthread_create(&threads[i], NULL, f, &arguments[i]);

        if (r) {
            printf("Eroare la crearea thread-ului %d\n", i);
            exit(-1);
        }
    }

    // Așteptăm terminarea thread-urilor
    for (i = 0; i < NUM_THREADS; i++) {
        r = pthread_join(threads[i], &status);

        if (r) {
            printf("Eroare la asteptarea thread-ului %d\n", i);
            exit(-1);
        }
    }

    // Distrugem mutexul
    pthread_mutex_destroy(&lock);

    printf("a = %d\n", a);

    return 0;
}
