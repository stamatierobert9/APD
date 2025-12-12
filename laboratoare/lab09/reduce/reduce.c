#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define MASTER 0

int main (int argc, char *argv[])
{
    int procs, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int value = rank;

    for (int i = 2; i <= procs * 2; i *= 2) {
        // TODO
        int step = i / 2;
        if (rank % i == 0) {
            int source = rank + step;
            if (source < procs) {
                int received_val;
                MPI_Recv(&received_val, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                value += received_val;
            }
        } else if (rank % i == step) {
            int dest = rank - step;
            MPI_Send(&value, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    }

    if (rank == MASTER) {
        printf("Result = %d\n", value);
    }

    MPI_Finalize();

}

