#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROOT 3

int main (int argc, char *argv[])
{
    int  numtasks, rank, len;
    char hostname[MPI_MAX_PROCESSOR_NAME];
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Get_processor_name(hostname, &len);

    int value;
    int tag = 0;

    if (numtasks < 4) {
        if (rank == 0) printf("Te rog ruleaza cu cel putin 4 procese (mpirun -np 4 ...)\n");
        MPI_Finalize();
        return 0;
    }

    if (rank == ROOT) {
        printf("Process [%d] (ROOT) waiting for messages...\n", rank);

        for (int i = 0; i < numtasks - 1; i++) {
            
            MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
            
            printf("ROOT: Received value %d from source rank %d.\n", value, status.MPI_SOURCE);
        }

    } else {
         // Generate a random number.
        srand(time(NULL) + rank);
        value = rand() % (rank * 50 + 1);

        printf("Process [%d] send %d.\n", rank, value);

        MPI_Send(&value, 1, MPI_INT, ROOT, tag, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}