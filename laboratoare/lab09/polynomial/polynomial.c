#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#define MASTER 0

int main(int argc, char * argv[]) {
    int rank;
    int nProcesses;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    printf("Hello from %i/%i\n", rank, nProcesses);

    if (rank == MASTER) { // This code is run by a single process
        int polynomialSize;
        int x = 5; // valoarea cu care se calculeaza polinomul - f(5)
        
        /*
            in fisierul de intrare formatul este urmatorul:
            numarul_de_coeficienti
            coeficient x^0
            coeficient x^1
            etc.
        */

        // Verificare existență fișier (opțional, dar recomandat)
        if (argc < 2) {
             MPI_Finalize();
             return 0;
        }

        FILE * polFunctionFile = fopen(argv[1], "rt");
        fscanf(polFunctionFile, "%d", &polynomialSize);
        /*
            in array-ul a se vor salva coeficientii ecuatiei / polinomului
            de exemplu: a = {1, 4, 4} => 1 * (x ^ 2) + 4 * (x ^ 1) + 4 * (x ^ 0)
        */
        float *a = malloc(sizeof(float)*polynomialSize);
        for (int i = 0; i < polynomialSize; i++) {
            fscanf(polFunctionFile, "%f", &a[i]);
            printf("Read value %f\n", a[i]);
            /*
                Se trimit coeficientii pentru x^1, x^2 etc. proceselor 1, 2 etc.
                Procesul 0 se ocupa de x^0 si are valoarea coeficientului lui x^0
            */
            if (i > 0) {
                    MPI_Send(&a[i], 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD);
                }
        }

        fclose(polFunctionFile);
        
        float current_sum = a[0]; 

        // Trimitem suma parțială (Tag 1) către procesul 1
        MPI_Send(&current_sum, 1, MPI_FLOAT, 1, 1, MPI_COMM_WORLD);
        
        // Trimitem valoarea X (Tag 2) către procesul 1
        MPI_Send(&x, 1, MPI_INT, 1, 2, MPI_COMM_WORLD);
        
        free(a); // Curățăm memoria

    } else {
        // --- CORECTURA ESTE AICI ---
        // Am schimbat 'val' in 'val_coeff' si 'x' in 'x_val' pentru a potrivi cu logica de mai jos
        float val_coeff, sum;
        int x_val; 

        /*
            se primesc: 
            - coeficientul corespunzator procesului (exemplu procesul 1 primeste coeficientul lui x^1)
            - suma partiala
            - valoarea x din f(x)
            si se calculeaza valoarea corespunzatoare pentru c * x^r, r fiind rangul procesului curent
            si c fiind coeficientul lui x^r, si se aduna la suma
        */
        // 1. Primim coeficientul de la Master (sursa: MASTER, tag: 0)
        MPI_Recv(&val_coeff, 1, MPI_FLOAT, MASTER, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 2. Primim suma parțială de la procesul din stânga (sursa: rank-1, tag: 1)
        MPI_Recv(&sum, 1, MPI_FLOAT, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // 3. Primim valoarea X de la procesul din stânga (sursa: rank-1, tag: 2)
        MPI_Recv(&x_val, 1, MPI_INT, rank - 1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 4. Calculăm: termenul curent = coeficient * (x ^ rank)
        // Adăugăm la suma primită
        float term = val_coeff * pow(x_val, rank);
        sum += term;

        if (rank == nProcesses - 1) {
            printf("Polynom value is %f\n", sum);
        } else {
            // se trimit x si suma partiala catre urmatorul proces
            MPI_Send(&sum, 1, MPI_FLOAT, rank + 1, 1, MPI_COMM_WORLD);
            MPI_Send(&x_val, 1, MPI_INT, rank + 1, 2, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}