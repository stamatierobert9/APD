#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#define N 1000
#define MASTER 0

void compareVectors(int * a, int * b) {
    // DO NOT MODIFY
    int i;
    for(i = 0; i < N; i++) {
        if(a[i]!=b[i]) {
            printf("Sorted incorrectly\n");
            return;
        }
    }
    printf("Sorted correctly\n");
}

void displayVector(int * v) {
    // DO NOT MODIFY
    int i;
    int displayWidth = 2 + log10(v[N-1]);
    for(i = 0; i < N; i++) {
        printf("%*i", displayWidth, v[i]);
    }
    printf("\n");
}

int cmp(const void *a, const void *b) {
    // DO NOT MODIFY
    int A = *(int*)a;
    int B = *(int*)b;
    return A-B;
}
 
int main(int argc, char * argv[]) {
    int rank, i, j;
    int nProcesses;
    MPI_Init(&argc, &argv);
    
    // Vectorul de pozitii (ranguri)
    int pos[N];
    int *v = (int*)malloc(sizeof(int)*N);
    int *vQSort = (int*)malloc(sizeof(int)*N);

    // Initializam pozitiile cu 0
    for (i = 0; i < N; i++)
        pos[i] = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    printf("Hello from %i/%i\n", rank, nProcesses);

    if (rank == MASTER) {
        // generate random vector
        srandom(42);
        for(i = 0; i < N; i++) {
            v[i] = random() % 10000; // Valori aleatoare
        }
    }

    // 1. Trimitem vectorul complet v catre toate procesele
    // Pentru a calcula rangul, fiecare proces trebuie sa stie toate numerele
    MPI_Bcast(v, N, MPI_INT, MASTER, MPI_COMM_WORLD);

    // Calculam intervalul de care este responsabil procesul curent
    // start si end definesc "bucata" din vector pentru care acest proces caluleaza rangul
    int start = rank * N / nProcesses;
    int end = (rank + 1) * N / nProcesses;

    if(rank == 0) {
        // DO NOT MODIFY
        // displayVector(v); // Comentat pentru ca la N=1000 umple consola

        // make copy to check it against qsort
        // DO NOT MODIFY
        for(i = 0; i < N; i++)
            vQSort[i] = v[i];
        qsort(vQSort, N, sizeof(int), cmp);

        // --- SORTAREA (MASTER) ---
        
        // A. Masterul calculeaza rangurile pentru bucata lui (start -> end)
        for (i = start; i < end; i++) {
            for (j = 0; j < N; j++) {
                // Dacă găsim un element mai mic, creștem rangul elementului curent v[i]
                // Condiția (v[j] == v[i] && j < i) tratează duplicatele (stabilitate)
                if (v[j] < v[i] || (v[j] == v[i] && j < i)) {
                    pos[i]++;
                }
            }
        }

        // B. Masterul primeste rangurile calculate de celelalte procese
        for (int p = 1; p < nProcesses; p++) {
            int p_start = p * N / nProcesses;
            int p_end = (p + 1) * N / nProcesses;
            int count = p_end - p_start;
            
            // Primim direct in array-ul pos la offset-ul corect
            MPI_Recv(&pos[p_start], count, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        // C. Reconstruim vectorul sortat folosind rangurile
        int *vSorted = (int*)malloc(sizeof(int) * N);
        for(i = 0; i < N; i++) {
            vSorted[pos[i]] = v[i]; // Punem elementul v[i] pe pozitia calculata pos[i]
        }
        
        // Copiem inapoi in v pentru verificare
        for(i = 0; i < N; i++) {
            v[i] = vSorted[i];
        }
        free(vSorted);

        // displayVector(v);
        compareVectors(v, vQSort);

    } else {
        
        // compute the positions
        // Procesele worker calculeaza rangurile pentru bucata lor
        for (i = start; i < end; i++) {
            for (j = 0; j < N; j++) {
                // Aceeasi logica de comparatie ca la Master
                if (v[j] < v[i] || (v[j] == v[i] && j < i)) {
                    pos[i]++;
                }
            }
        }

        int count = end - start;
        MPI_Send(&pos[start], count, MPI_INT, MASTER, 0, MPI_COMM_WORLD);
    }

    free(v);
    free(vQSort);

    MPI_Finalize();
    return 0;
}