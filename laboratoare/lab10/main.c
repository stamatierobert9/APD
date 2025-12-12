#include<mpi.h>
#include<stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define CONVERGENCE_COEF 100
#define TAG_SONDA 0
#define TAG_ECOU 1

/**
 * Run: mpirun --oversubscribe -np 12 ./a.out
 */

static int num_neigh;
static int *neigh;

void read_neighbours(int rank) {
    FILE *fp;
    char file_name[15];
    sprintf(file_name, "./files/%d.in", rank);

    fp = fopen(file_name, "r");
    // Fallback: incercam in directorul curent daca nu exista folderul files
    if (!fp) {
        sprintf(file_name, "%d.in", rank);
        fp = fopen(file_name, "r");
    }

    fscanf(fp, "%d", &num_neigh);

    neigh = malloc(sizeof(int) * num_neigh);

    for (size_t i = 0; i < num_neigh; i++)
        fscanf(fp, "%d", &neigh[i]);
    
    fclose(fp);
}

int* get_dst(int rank, int numProcs, int leader) {
    MPI_Status status;
    MPI_Request request;

    /* Vectori de parinti */
    int *v = malloc(sizeof(int) * numProcs);
    int *vRecv = malloc(sizeof(int) * numProcs);
    /* O valoare aleatoare pentru a fi folosita ca sonda.
     * MPI permite și mesaje de lungime 0, dar pentru 
         * a da mai multa claritate codului vom folosi aceasta valoare.
    */ 
    int sonda = 42;

    memset(v, -1, sizeof(int) * numProcs);
    memset(vRecv, -1, sizeof(int) * numProcs);
    
    if (rank == leader)
        v[rank] = -1;
    else {
        /* Daca procesul curent nu este liderul, inseamna ca va astepta un mesaj de la un parinte */
        MPI_Recv(&sonda, 1, MPI_INT, MPI_ANY_SOURCE, TAG_SONDA, MPI_COMM_WORLD, &status);
        v[rank] = status.MPI_SOURCE;
    }


    /*
    * TODO2: Pentru fiecare proces vecin care nu este parintele procesului curent,
    * voi trimite o sonda. 
    */
    for (int i = 0; i < num_neigh; i++) {
        if (neigh[i] != v[rank]) {
            MPI_Send(&sonda, 1, MPI_INT, neigh[i], TAG_SONDA, MPI_COMM_WORLD);
        }
    }

    /*
    * TODO2: Vom astepta de la fiecare proces vecin care nu este parintele procesului curent vectorul de parinti sau o sonda.
            Daca primim un ecou (vector de parinti), actualizam vectorul propriu de parinti daca exista informatii aditionale.
        HINT: Pentru simplitate, puteti face mereu recv ca pentru vectorul de parinti si sa verificati size-ul receptiei sau tag-ul
            pentru a determina daca este sonda sau ecou.
    */
    for (int i = 0; i < num_neigh; i++) {
        if (neigh[i] != v[rank]) {
            // Asteptam orice mesaj de la vecini (Sonda de explorare sau Ecou cu date)
            MPI_Recv(vRecv, numProcs, MPI_INT, neigh[i], MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_ECOU) {
                // Am primit date de la un copil -> Combinam vectorii de parinti
                for (int k = 0; k < numProcs; k++) {
                    if (vRecv[k] != -1) {
                        v[k] = vRecv[k];
                    }
                }
            }
            // Daca e TAG_SONDA, o ignoram (e doar o bucla in graf, nu face parte din arbore)
        }
    }

    /*
    * TODO2: Orice proces ce nu este lider va propaga vectorul de vecini parintelui lui si va astepta topologia completa de la acesta
    */
    if (rank != leader) {
        // Trimitem ce am adunat pana acum parintelui (Ecou)
        MPI_Send(v, numProcs, MPI_INT, v[rank], TAG_ECOU, MPI_COMM_WORLD);
        
        // Asteptam vectorul FINAL de parinti de la parinte (pentru a sti cine ne sunt copiii la pasul urmator)
        MPI_Recv(v, numProcs, MPI_INT, v[rank], TAG_ECOU, MPI_COMM_WORLD, &status);
    }


    /*
    * TODO2: Procesul curent va trimite doar copiilor lui topologia completa
    */
    // Distribuim vectorul complet de parinti catre copii
    for (int i = 0; i < numProcs; i++) {
        if (v[i] == rank) { // Daca 'i' ma are pe mine ca parinte, e copilul meu
             MPI_Send(v, numProcs, MPI_INT, i, TAG_ECOU, MPI_COMM_WORLD);
        }
    }

    for (int i = 0; i < numProcs && rank == leader; i++) {
        printf("The node %d has the parent %d\n", i, v[i]);
    }

    free(vRecv);
    return v;
}

int leader_chosing(int rank, int nProcesses) {
    int leader = -1;
    int q;
    leader = rank;
    int recv_leader;
    MPI_Status status;
    
    /* Executam acest pas pana ajungem la convergenta */
    for (int k = 0; k < CONVERGENCE_COEF; k++) {
        /* TODO1: Pentru fiecare vecin, vom trimite liderul pe care il cunosc 
        * si voi astepta un mesaj de la orice vecin
        * Daca liderul e mai mare decat al meu, il actualizez pe al meu
        */
        
        // Etapa 1: Trimitere
        for (int i = 0; i < num_neigh; i++) {
            MPI_Send(&leader, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
        }

        // Etapa 2: Primire si actualizare
        for (int i = 0; i < num_neigh; i++) {
            MPI_Recv(&recv_leader, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
            if (recv_leader > leader) {
                leader = recv_leader;
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    printf("%i/%i: leader is %i\n", rank, nProcesses, leader);

    return leader;
}

int get_number_of_nodes(int rank, int leader) {
    
    double val;
    if (leader == rank) {
        val = 1.0;
    } else {
        val = 0.0;
    }

    double recvd = 0;
    MPI_Status status;

    /* Executam acest pas pana ajungem la convergenta */
    for (int k = 0; k < CONVERGENCE_COEF; k++) {
        /* TODO3: Pentru fiecare vecin, vom trimite valoarea pe care o cunosc
        * si voi astepta un mesaj de la el
        * Cu valoarea primita, actualizam valoarea cunoscuta ca fiind
        * media dintre cele 2
        */
        for (int i = 0; i < num_neigh; i++) {
            // Facem schimb pereche cu fiecare vecin
            MPI_Send(&val, 1, MPI_DOUBLE, neigh[i], 0, MPI_COMM_WORLD);
            MPI_Recv(&recvd, 1, MPI_DOUBLE, neigh[i], 0, MPI_COMM_WORLD, &status);
            
            val = (val + recvd) / 2.0;
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    // Inversam valoarea pentru a obtine numarul de noduri (aproximativ)
    return (int)(round(1.0 / val));
}

int ** get_topology(int rank, int nProcesses, int * parents, int leader) {
    int ** topology = malloc(sizeof(int*) * nProcesses);
    int ** vTopology = malloc(sizeof(int*) * nProcesses);
    
    for (size_t i = 0; i < nProcesses; i++) {
        topology[i] = calloc(sizeof(int), nProcesses);
        vTopology[i] = calloc(sizeof(int), nProcesses); // Buffer temp
    }

    for (size_t i = 0; i < num_neigh; i++) {
        topology[rank][neigh[i]] = 1;
    }

    MPI_Status status;

    /* TODO4: Primim informatii de la toti copii si actualizam matricea de topologie */
    // Identificam copiii folosind vectorul parents calculat anterior
    for (int i = 0; i < nProcesses; i++) {
        if (parents[i] == rank) { // i este copilul meu
            // Primim liniile matricei de la copil
            for (int r = 0; r < nProcesses; r++) {
                MPI_Recv(vTopology[r], nProcesses, MPI_INT, i, 2, MPI_COMM_WORLD, &status);
                // Facem reuniunea (merge) matricelor
                for (int c = 0; c < nProcesses; c++) {
                    if (vTopology[r][c] == 1) {
                        topology[r][c] = 1;
                    }
                }
            }
        }
    }

    /* TODO4: Propagam matricea proprie catre parinte */
    if (rank != leader) {
        // Trimitem matricea curenta (care include si datele de la copii) catre parinte
        for (int r = 0; r < nProcesses; r++) {
            MPI_Send(topology[r], nProcesses, MPI_INT, parents[rank], 2, MPI_COMM_WORLD);
        }
    }

    /* TODO4: Daca nu suntem liderul, asteptam topologia completa de la parinte  */
    if (rank != leader) {
        for (int r = 0; r < nProcesses; r++) {
            MPI_Recv(topology[r], nProcesses, MPI_INT, parents[rank], 3, MPI_COMM_WORLD, &status);
        }
    }
    
    /* TODO4: Trimitem topologia completa copiilor */
    for (int i = 0; i < nProcesses; i++) {
        if (parents[i] == rank) { // pentru fiecare copil
             for (int r = 0; r < nProcesses; r++) {
                MPI_Send(topology[r], nProcesses, MPI_INT, i, 3, MPI_COMM_WORLD);
            }
        }
    }

    // Curatenie memorie temporara
    for (size_t i = 0; i < nProcesses; i++) free(vTopology[i]);
    free(vTopology);

    return topology;
}

int main(int argc, char * argv[]) {
    int rank, nProcesses, num_procs, leader;
    int *parents, **topology;

    MPI_Init(&argc, &argv);
    MPI_Status status;
    MPI_Request request;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    // Ajustare pentru a rula cu numarul corect de procese cerut de input-uri (exemplu 12)
    // Daca e nevoie de flexibilitate, comentati verificarea
    if (nProcesses != 12) {
        printf("please run with: mpirun --oversubscribe -np 12 %s\n", argv[0]);
        MPI_Finalize(); 
        exit(0);
    }
     
    read_neighbours(rank);
    leader = leader_chosing(rank, nProcesses);
    
    MPI_Barrier(MPI_COMM_WORLD);

    parents = get_dst(rank, nProcesses, leader);

    MPI_Barrier(MPI_COMM_WORLD);

    num_procs = get_number_of_nodes(rank, leader);
    
    printf("%d/%d There are %d processes\n", rank, nProcesses,num_procs);

    topology = get_topology(rank, nProcesses, parents, leader);

    for (size_t i = 0; i < nProcesses && rank == leader; i++) // Am modificat rank == 0 cu rank == leader pentru a fi corect
    {
        for (size_t j = 0; j < nProcesses; j++)
        {
            printf("%2d ", topology[i][j]); 
        }
        printf("\n");
    }
    
    MPI_Finalize();
    return 0;
}