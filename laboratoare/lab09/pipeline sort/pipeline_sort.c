#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>

int N;

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
    for(i = 0; i < N; i++) {
        printf("%d ", v[i]);
    }
    printf("\n");
}

int cmp(const void *a, const void *b) {
    // DO NOT MODIFY
    int A = *(int*)a;
    int B = *(int*)b;
    return A-B;
}

// Use 'mpirun -np 20 --oversubscribe ./pipeline_sort' to run the application with more processes
int main(int argc, char * argv[]) {
    int rank;
    int nProcesses;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    
    // Numarul total de elemente de sortat este nProcesses - 1
    // Deoarece Rank 0 este doar coordonator.
    int numElements = nProcesses - 1;

    if(rank==0) { // This code is run by a single process
        int *v = (int*)malloc(sizeof(int) * numElements);
        int *vQSort = (int*)malloc(sizeof(int) * numElements);
        int i;

        // generate the vector v with random values
        // DO NOT MODIFY
        srandom(42);
        for(i = 0; i < numElements; i++)
            v[i] = random() % 200;
        N = numElements;
        displayVector(v);

        // make copy to check it against qsort
        // DO NOT MODIFY
        for(i = 0; i < numElements; i++)
            vQSort[i] = v[i];
        qsort(vQSort, numElements, sizeof(int), cmp);

        // TODO send the vector to rank == 1
        // Masterul trimite numerele unul cate unul in "teava" (catre Rank 1)
        for (i = 0; i < numElements; i++) {
            MPI_Send(&v[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        }

        // Recuperam valorile sortate.
        // Rank 1 va avea cel mai mic numar, Rank 2 urmatorul, etc.
        for (i = 0; i < numElements; i++) {
            MPI_Recv(&v[i], 1, MPI_INT, i + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        displayVector(v);
        compareVectors(v, vQSort);
        
        free(v);
        free(vQSort);

    } else {
        // TODO sort the vector v using N processes (N == nProcesses - 1)
        
        int stored_value = -1;
        int received_value;
        int has_value = 0; // flag sa stim daca avem deja o valoare stocata

        // Calculam cate numere vor trece prin acest proces.
        // Rank 1 primeste toate N numerele.
        // Rank 2 primeste N-1 numere (unul a ramas la Rank 1).
        // Rank k primeste (nProcesses - k) numere.
        int items_to_receive = nProcesses - rank;

        for (int i = 0; i < items_to_receive; i++) {
            // Primim numarul de la stanga (rank - 1)
            MPI_Recv(&received_value, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (!has_value) {
                // Daca nu avem nicio valoare stocata, o pastram pe aceasta
                stored_value = received_value;
                has_value = 1;
            } else {
                // Daca avem deja o valoare, comparam
                if (received_value < stored_value) {
                    // Daca ce am primit e mai mic decat ce am, pastram ce am primit
                    // si trimitem mai departe ce aveam inainte (pentru ca e mai mare)
                    MPI_Send(&stored_value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                    stored_value = received_value;
                } else {
                    // Daca ce am primit e mai mare, il trimitem mai departe
                    MPI_Send(&received_value, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
                }
            }
        }

        // Dupa ce s-a terminat sortarea, trimitem valoarea noastra inapoi la Master (Rank 0)
        // pentru afisare
        MPI_Send(&stored_value, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}