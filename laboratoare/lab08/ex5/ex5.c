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

    // Checks the number of processes allowed.
    if (numtasks != 2) {
        if (rank == 0) printf("Wrong number of processes. Only 2 allowed! Use: mpirun -np 2 ./ex5\n");
        MPI_Finalize();
        return 0;
    }

    int send_numbers = 10;
    
    srand(time(NULL) + rank);

    if (rank == 0) {
        // Generate the random numbers and tags.
        for (int i = 0; i < send_numbers; i++) {
            int value = rand() % 100;
            int tag = (rand() % 50) + 1;
            
            printf("Rank 0: Trimit valoarea %d cu tag-ul %d\n", value, tag);
            
            MPI_Send(&value, 1, MPI_INT, 1, tag, MPI_COMM_WORLD);
        }

    } else {
        int recv_value;
        MPI_Status status;

        // Receives the information from the first process.
        for (int i = 0; i < send_numbers; i++) {
            MPI_Recv(&recv_value, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            printf("Rank 1: Am primit valoarea %d cu tag-ul %d (Status.MPI_TAG)\n", 
                   recv_value, status.MPI_TAG);
        }
    }

    MPI_Finalize();
    return 0;
}