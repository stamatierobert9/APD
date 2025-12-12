#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define GROUP_SIZE 4

int main (int argc, char *argv[])
{
    int old_size, new_size;
    int old_rank, new_rank;
    int recv_rank;
    MPI_Comm custom_group;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &old_size); // Total number of processes.
    MPI_Comm_rank(MPI_COMM_WORLD, &old_rank); // The current process ID / Rank.

    if (old_size % GROUP_SIZE != 0) {
        if (old_rank == 0) {
            printf("Eroare: Numarul de procese (%d) trebuie sa fie divizibil cu %d.\n", old_size, GROUP_SIZE);
            printf("Incearca: mpirun -np 8 ./ex6\n");
        }
        MPI_Finalize();
        return 0;
    }

    int color = old_rank / GROUP_SIZE;
    int key = old_rank; 

    MPI_Comm_split(MPI_COMM_WORLD, color, key, &custom_group);

    MPI_Comm_rank(custom_group, &new_rank);
    MPI_Comm_size(custom_group, &new_size);

    printf("Rank [%d] / size [%d] in MPI_COMM_WORLD and rank [%d] / size [%d] in custom group [%d].\n",
            old_rank, old_size, new_rank, new_size, color);

    int next_rank = (new_rank + 1) % new_size;
    int prev_rank = (new_rank - 1 + new_size) % new_size;

    MPI_Sendrecv(&new_rank, 1, MPI_INT, next_rank, 0,
                 &recv_rank, 1, MPI_INT, prev_rank, 0,
                 custom_group, MPI_STATUS_IGNORE);

    printf("Process [%d] from group [%d] received [%d].\n", new_rank,
            color, recv_rank);

    MPI_Comm_free(&custom_group);

    MPI_Finalize();
    return 0;
}