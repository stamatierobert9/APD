#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include "pthread_barrier_mac.h"

/* ===== util: alocări matrici contigue ===== */
static int** mat_alloc(int n) {
    int **m = (int**)malloc(n * sizeof(*m));
    if (!m) { perror("malloc m"); exit(1); }
    m[0] = (int*)calloc((size_t)n * n, sizeof(int));
    if (!m[0]) { perror("calloc m[0]"); exit(1); }
    for (int i = 1; i < n; i++) m[i] = m[0] + i * n;
    return m;
}
static void mat_free(int **m) {
    if (!m) return;
    free(m[0]); free(m);
}
static void mat_copy(int **dst, int **src, int n) {
    memcpy(dst[0], src[0], (size_t)n*n*sizeof(int));
}
static void mat_add(int **A, int **B, int **C, int n) {
    int *a=A[0], *b=B[0], *c=C[0];
    for (int i=0;i<n*n;i++) c[i] = a[i] + b[i];
}
static void mat_sub(int **A, int **B, int **C, int n) {
    int *a=A[0], *b=B[0], *c=C[0];
    for (int i=0;i<n*n;i++) c[i] = a[i] - b[i];
}
static void mat_mul_naive(int **A, int **B, int **C, int n) {
    // C += A*B (dar noi folosim pe zero, deci e C = A*B)
    for (int i=0;i<n;i++) {
        for (int k=0;k<n;k++) {
            int aik = A[i][k];
            for (int j=0;j<n;j++) {
                C[i][j] += aik * B[k][j];
            }
        }
    }
}

/* ===== împărțire în blocuri ===== */
typedef struct { int **a11, **a12, **a21, **a22; } Quad;
static Quad split_quads(int **A, int n) {
    int m = n/2;
    Quad q = {0};
    q.a11 = mat_alloc(m);
    q.a12 = mat_alloc(m);
    q.a21 = mat_alloc(m);
    q.a22 = mat_alloc(m);
    for (int i=0;i<m;i++) {
        memcpy(q.a11[i], A[i], m*sizeof(int));
        memcpy(q.a12[i], A[i] + m, m*sizeof(int));
        memcpy(q.a21[i], A[i+m], m*sizeof(int));
        memcpy(q.a22[i], A[i+m] + m, m*sizeof(int));
    }
    return q;
}
static void join_quads(int **C, Quad q, int n) {
    int m = n/2;
    for (int i=0;i<m;i++) {
        memcpy(C[i],          q.a11[i], m*sizeof(int));
        memcpy(C[i] + m,      q.a12[i], m*sizeof(int));
        memcpy(C[i+m],        q.a21[i], m*sizeof(int));
        memcpy(C[i+m] + m,    q.a22[i], m*sizeof(int));
    }
}
static void free_quad(Quad q) {
    mat_free(q.a11); mat_free(q.a12); mat_free(q.a21); mat_free(q.a22);
}

/* ===== Strassen serial (recursiv) ===== */
static const int STRASSEN_THRESHOLD = 64; // poți ajusta

static void strassen_serial(int **A, int **B, int **C, int n) {
    if (n <= STRASSEN_THRESHOLD) {
        mat_mul_naive(A, B, C, n);
        return;
    }
    if (n & 1) { // fallback simplu, nu ar trebui să apară (padăm la putere de 2)
        mat_mul_naive(A, B, C, n);
        return;
    }

    int m = n/2;
    Quad a = split_quads(A, n);
    Quad b = split_quads(B, n);

    // M1..M7
    int **M1 = mat_alloc(m), **M2 = mat_alloc(m), **M3 = mat_alloc(m),
        **M4 = mat_alloc(m), **M5 = mat_alloc(m), **M6 = mat_alloc(m), **M7 = mat_alloc(m);
    int **T1 = mat_alloc(m), **T2 = mat_alloc(m);

    // M1 = (A11 + A22) * (B11 + B22)
    mat_add(a.a11, a.a22, T1, m);
    mat_add(b.a11, b.a22, T2, m);
    strassen_serial(T1, T2, M1, m);

    // M2 = (A21 + A22) * B11
    mat_add(a.a21, a.a22, T1, m);
    strassen_serial(T1, b.a11, M2, m);

    // M3 = A11 * (B12 - B22)
    mat_sub(b.a12, b.a22, T2, m);
    strassen_serial(a.a11, T2, M3, m);

    // M4 = A22 * (B21 - B11)
    mat_sub(b.a21, b.a11, T2, m);
    strassen_serial(a.a22, T2, M4, m);

    // M5 = (A11 + A12) * B22
    mat_add(a.a11, a.a12, T1, m);
    strassen_serial(T1, b.a22, M5, m);

    // M6 = (A21 - A11) * (B11 + B12)
    mat_sub(a.a21, a.a11, T1, m);
    mat_add(b.a11, b.a12, T2, m);
    strassen_serial(T1, T2, M6, m);

    // M7 = (A12 - A22) * (B21 + B22)
    mat_sub(a.a12, a.a22, T1, m);
    mat_add(b.a21, b.a22, T2, m);
    strassen_serial(T1, T2, M7, m);

    // C11 = M1 + M4 - M5 + M7
    // C12 = M3 + M5
    // C21 = M2 + M4
    // C22 = M1 - M2 + M3 + M6
    Quad c;
    c.a11 = mat_alloc(m); c.a12 = mat_alloc(m); c.a21 = mat_alloc(m); c.a22 = mat_alloc(m);

    int **U = mat_alloc(m);
    mat_add(M1, M4, U, m);        // U = M1 + M4
    mat_sub(U, M5, U, m);         // U = U  - M5
    mat_add(U, M7, c.a11, m);     // C11

    mat_add(M3, M5, c.a12, m);    // C12
    mat_add(M2, M4, c.a21, m);    // C21

    mat_sub(M1, M2, U, m);        // U = M1 - M2
    mat_add(U, M3, U, m);         // U = U + M3
    mat_add(U, M6, c.a22, m);     // C22

    join_quads(C, c, n);

    // free temporare
    mat_free(U);
    mat_free(T1); mat_free(T2);
    mat_free(M1); mat_free(M2); mat_free(M3); mat_free(M4); mat_free(M5); mat_free(M6); mat_free(M7);
    free_quad(a); free_quad(b); free_quad(c);
}

/* ===== infrastructură paralelă (top-level Strassen jobs) ===== */

typedef struct {
    int id;          // 0..P-1
} ThreadInfo;

/* date globale partajate între threaduri */
static int **A, **B, **C;    // matrici padate la n2
static int N_in, n2, P;

static pthread_mutex_t q_mtx = PTHREAD_MUTEX_INITIALIZER;
static int next_job = 1; // 1..7 => M1..M7

static pthread_barrier_t barrier;

static int **M[8]; // M[1]..M[7] rezultate (m x m), partajate
static int    msz; // m = n2/2

/* ia următorul job din coadă (1..7) sau 0 dacă s-au terminat */
static int fetch_job() {
    int job = 0;
    pthread_mutex_lock(&q_mtx);
    if (next_job <= 7) job = next_job++;
    pthread_mutex_unlock(&q_mtx);
    return job;
}

static void compute_Mk(int k) {
    // pregătește submatrici A11.. , B11.. la nivel de top
    int n = n2, m = n/2;
    // extrage blocuri: copiem în matrici m×m (cost O(n^2), amortizat vs O(n^3))
    Quad a = split_quads(A, n);
    Quad b = split_quads(B, n);

    int **Mk = mat_alloc(m);
    int **T1 = mat_alloc(m), **T2 = mat_alloc(m);

    switch (k) {
        case 1: // (A11 + A22)*(B11 + B22)
            mat_add(a.a11, a.a22, T1, m);
            mat_add(b.a11, b.a22, T2, m);
            strassen_serial(T1, T2, Mk, m);
            break;
        case 2: // (A21 + A22)*B11
            mat_add(a.a21, a.a22, T1, m);
            strassen_serial(T1, b.a11, Mk, m);
            break;
        case 3: // A11*(B12 - B22)
            mat_sub(b.a12, b.a22, T2, m);
            strassen_serial(a.a11, T2, Mk, m);
            break;
        case 4: // A22*(B21 - B11)
            mat_sub(b.a21, b.a11, T2, m);
            strassen_serial(a.a22, T2, Mk, m);
            break;
        case 5: // (A11 + A12)*B22
            mat_add(a.a11, a.a12, T1, m);
            strassen_serial(T1, b.a22, Mk, m);
            break;
        case 6: // (A21 - A11)*(B11 + B12)
            mat_sub(a.a21, a.a11, T1, m);
            mat_add(b.a11, b.a12, T2, m);
            strassen_serial(T1, T2, Mk, m);
            break;
        case 7: // (A12 - A22)*(B21 + B22)
            mat_sub(a.a12, a.a22, T1, m);
            mat_add(b.a21, b.a22, T2, m);
            strassen_serial(T1, T2, Mk, m);
            break;
        default: break;
    }

    M[k] = Mk;
    mat_free(T1); mat_free(T2);
    free_quad(a); free_quad(b);
}

static void* worker(void *arg) {
    ThreadInfo *ti = (ThreadInfo*)arg;
    // 1) consumă joburile M1..M7
    for (;;) {
        int k = fetch_job();
        if (k == 0) break;
        compute_Mk(k);
    }

    // 2) barieră: așteaptă ca toate M1..M7 să fie gata
    pthread_barrier_wait(&barrier);

    // 3) thread-ul 0 combină C și afișează (main nu are voie)
    if (ti->id == 0) {
        int m = msz;
        // C11 = M1 + M4 - M5 + M7
        // C12 = M3 + M5
        // C21 = M2 + M4
        // C22 = M1 - M2 + M3 + M6
        Quad c;
        c.a11 = mat_alloc(m); c.a12 = mat_alloc(m); c.a21 = mat_alloc(m); c.a22 = mat_alloc(m);
        int **U = mat_alloc(m);

        mat_add(M[1], M[4], U, m);
        mat_sub(U,   M[5], U, m);
        mat_add(U,   M[7], c.a11, m);

        mat_add(M[3], M[5], c.a12, m);
        mat_add(M[2], M[4], c.a21, m);

        mat_sub(M[1], M[2], U, m);
        mat_add(U,   M[3], U, m);
        mat_add(U,   M[6], c.a22, m);

        join_quads(C, c, n2);

        // afișare doar partea N_in x N_in
        for (int i=0;i<N_in;i++) {
            for (int j=0;j<N_in;j++) {
                printf("%d ", C[i][j]);
            }
            printf("\n");
        }

        // curățenie
        mat_free(U);
        free_quad(c);
        for (int k=1;k<=7;k++) { mat_free(M[k]); M[k]=NULL; }
    }

    return NULL;
}

/* next power of two >= x */
static int next_pow2(int x) {
    int p=1; while (p < x) p <<= 1; return p;
}

/* ===== main ===== */
int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Utilizare: %s N P\n", argv[0]);
        return 1;
    }
    N_in = atoi(argv[1]);
    P    = atoi(argv[2]);
    if (N_in <= 0 || P < 1 || P > 7) {
        fprintf(stderr, "N>0 si P in {1..7}\n");
        return 1;
    }

    n2 = next_pow2(N_in);
    A = mat_alloc(n2); B = mat_alloc(n2); C = mat_alloc(n2);

    // citire A si B (N_in x N_in), restul pad cu 0
    for (int i=0;i<N_in;i++)
        for (int j=0;j<N_in;j++) scanf("%d", &A[i][j]);
    for (int i=0;i<N_in;i++)
        for (int j=0;j<N_in;j++) scanf("%d", &B[i][j]);

    // pregătiri globale
    msz = n2/2;
    next_job = 1;

    // barieră pentru P threaduri
    pthread_barrier_init(&barrier, NULL, P);

    // pornește P thread-uri o singură dată
    pthread_t *th = (pthread_t*)malloc(P * sizeof(*th));
    ThreadInfo *info = (ThreadInfo*)malloc(P * sizeof(*info));
    for (int i=0;i<P;i++) {
        info[i].id = i;
        if (pthread_create(&th[i], NULL, worker, &info[i]) != 0) {
            perror("pthread_create"); return 1;
        }
    }

    // un singur join (o etapă)
    for (int i=0;i<P;i++) pthread_join(th[i], NULL);

    pthread_barrier_destroy(&barrier);
    free(info); free(th);

    mat_free(A); mat_free(B); mat_free(C);
    return 0;
}
