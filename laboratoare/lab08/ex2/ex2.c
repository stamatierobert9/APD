#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROOT 0

int main (int argc, char *argv[])
{
    int  numtasks, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int rand_num;

    // Root process generates a random number.
    // Broadcasts to all processes.
    if (rank == ROOT) {
        srand(time(NULL));
        rand_num = rand() % 100; // Generăm un număr între 0 și 99
        printf("Process [%d] (ROOT) generated value: %d\n", rank, rand_num);
    }

    MPI_Bcast(&rand_num, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    printf("Process [%d], after broadcast %d.\n", rank, rand_num);

    MPI_Finalize();

}

