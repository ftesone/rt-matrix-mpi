#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

#define COORD_ID 0;
#define TAG_DEFAULT 0;

int main (int argc, char *argv[]) {
    int id, size, workers, blocksize;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    workers = size-1;

    if (N%workers != 0) {
        printf("No se puede distribuir el trabajo de forma equitativa entre los procesos\n");

        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_SIZE);

        return EXIT_FAILURE;
    }

    blocksize = N/workers; // cantidad de filas que va a calcular cada worker

    double *a, *b, *c;

    if (id == COORD_ID) { // coordinador
        a = (double *) malloc(sizeof(double) * N * N);
        b = (double *) malloc(sizeof(double) * N * N);
        c = (double *) malloc(sizeof(double) * N * N);

        for (int i=0 ; i<N ; i++) {
            for (int j=0 ; j<N ; j++) {
                a[i*N+j] = 1;
                b[j*N+i] = 1;
            }
        }

        clock_t begin = clock();

        // enviar los pedazos de A
        for (int i=0 ; i<workers ; i++) {
            MPI_Send(a[i*N*blocksize], N*blocksize, MPI_DOUBLE, i+1, TAG_DEFAULT, MPI_COMM_WORLD);
        }

        // enviar B[j]
        for (int j=0 ; j<N ; j++) {
            MPI_Send(b[j], N, MPI_DOUBLE, id+1, TAG_DEFAULT, MPI_COMM_WORLD);
        }

        for (int i=0 ; i<workers ; i++) {
            MPI_Recv(c[i*N*blocksize], N*blocksize, MPI_DOUBLE, MPI_ANY_SOURCE, TAG_DEFAULT, MPI_COMM_WORLD, &status);
        }

        clock_t end = clock();
        printf("Tiempo: %.4f\n", ((double)(end-begin)/CLOCKS_PER_SEC));

        if (N<32) {
            for (int i=0 ; i<N ; i++) {
                for (int j=0 ; j<N ; j++) {
                    printf("%4.2f", c[i*N+j]);
                }
                printf("\n");
            }
        }
    } else { // worker
        a = (double *) malloc(sizeof(double) * blocksize);
        b = (double *) malloc(sizeof(double) * N * N);
        c = (double *) malloc(sizeof(double) * blocksize);

        for (int i=0 ; i<blocksize ; i++) {
            c[i] = 0;
        }

        MPI_Recv(a, N*blocksize, MPI_DOUBLE, COORD_ID, TAG_DEFAULT, MPI_COMM_WORLD, &status);

        printf("worker[%d] recibió A\n", id);

        for (int j=0 ; j<N ; b++) {
            MPI_Recv(b[j*N], N, MPI_DOUBLE, MPI_ANY_SOURCE, TAG_DEFAULT, MPI_COMM_WORLD, &status);

            printf("worker[%d] recibió B[%d]\n", id, j);

            if ((id+1)%size != COORD_ID) {
                MPI_Send(b[j*N], N, MPI_DOUBLE, id+1, TAG_DEFAULT, MPI_COMM_WORLD);
            }

            for (int i=0 ; i<blocksize ; i++) {
                for (int k=0 ; k<N ; k++) {
                    c[i*N+j] += a[i*N+k] * b[j*N+k];
                }
            }
        }

        MPI_Send(c, N*blocksize, MPI_DOUBLE, COORD_ID, TAG_DEFAULT, MPI_COMM_WORLD);
    }

    free(a);
    free(b);
    free(c);

    MPI_Finalize();

    return EXIT_SUCCESS;
}
