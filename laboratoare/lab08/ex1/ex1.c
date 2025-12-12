#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[])
{
    int  numtasks, rank;
    int tag = 1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);

    int recv_num;

    // First process starts the circle.
    if (rank == 0) {
        // First process starts the circle.
        // Generate a random number.
        // Send the number to the next process.
        srand(time(NULL));
        int initial_num = rand() % 100;

        printf("Rank 0: Start - Trimit numarul %d catre Rank 1\n", initial_num);
        MPI_Send(&initial_num, 1, MPI_INT, 1, tag, MPI_COMM_WORLD);
        MPI_Recv(&recv_num, 1, MPI_INT, numtasks - 1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank 0: Final - Am primit inapoi valoarea %d de la Rank %d\n", recv_num, numtasks - 1);

    } else if (rank == numtasks - 1) {
        // Last process close the circle.
        // Receives the number from the previous process.
        // Increments the number.
        // Sends the number to the first process.
        MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("Rank %d (Ultimul): Am primit %d. Incrementez cu 2.\n", rank, recv_num);
        
        recv_num += 2;

        MPI_Send(&recv_num, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    } else {
        // Middle process.
        // Receives the number from the previous process.
        // Increments the number.
        // Sends the number to the next process.
        MPI_Recv(&recv_num, 1, MPI_INT, rank - 1, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("Rank %d: Am primit %d. Incrementez cu 2.\n", rank, recv_num);
        
        recv_num += 2;

        MPI_Send(&recv_num, 1, MPI_INT, rank + 1, tag, MPI_COMM_WORLD);

    }

    MPI_Finalize();
    return 0;

}

