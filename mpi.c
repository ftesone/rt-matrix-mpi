#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

#define COORD_ID 0
#define TAG_DEFAULT 0

void main (int argc, char *argv[]) {
    int id, size, workers, blocksize;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    workers = size-1;

    if (id == COORD_ID && N%workers != 0) {
        printf("No se puede distribuir el trabajo de forma equitativa entre los procesos\n");

        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_SIZE);
    }

    blocksize = N/workers; // cantidad de filas que va a calcular cada worker

    double *a, *b, *c;
    a = (double *) malloc(sizeof(double) * N * N);
    b = (double *) malloc(sizeof(double) * N * N);
    c = (double *) malloc(sizeof(double) * N * N);

    if (id == COORD_ID) { // coordinador
        for (int i=0 ; i<N ; i++) {
            for (int j=0 ; j<N ; j++) {
                a[i*N+j] = i*N+j+1;
                b[j*N+i] = i*N+j+1;
            }
        }

        clock_t begin = clock();

        // enviar los pedazos de A
        for (int i=0 ; i<workers ; i++) {
            MPI_Send(&a[i*N*blocksize], N*blocksize, MPI_DOUBLE, i+1, TAG_DEFAULT, MPI_COMM_WORLD);
        }

        // enviar B
#ifndef BLOCKS
        MPI_Bcast(b, N*N, MPI_DOUBLE, COORD_ID, MPI_COMM_WORLD);
#endif
#ifdef BLOCKS
        for (int i=0 ; i<workers ; i++) {
            for (int j=0 ; j<workers ; j++) {
                MPI_Send(&b[i*N*blocksize], N*blocksize, MPI_DOUBLE, j+1, TAG_DEFAULT, MPI_COMM_WORLD);
            }
        }
#endif

        for (int i=0 ; i<workers ; i++) {
            MPI_Recv(&c[i*N*blocksize], N*blocksize, MPI_DOUBLE, i+1, TAG_DEFAULT, MPI_COMM_WORLD, &status);
        }

        clock_t end = clock();
        printf("Tiempo: %.4f\n", ((double)(end-begin)/CLOCKS_PER_SEC));

        if (N<16) {
            for (int i=0 ; i<N ; i++) {
                for (int j=0 ; j<N ; j++) {
                    printf("%.2f", c[i*N+j]);
                }
                printf("\n");
            }
        }
    } else { // worker
        for (int i=0 ; i<N*blocksize ; i++) {
            c[i] = 0;
        }

        MPI_Recv(a, N*blocksize, MPI_DOUBLE, COORD_ID, TAG_DEFAULT, MPI_COMM_WORLD, &status);

#ifndef BLOCKS
        MPI_Bcast(b, N*N, MPI_DOUBLE, COORD_ID, MPI_COMM_WORLD);
        for (int i=0 ; i<blocksize ; i++) {
            for (int j=0 ; j<N ; j++) {
                for (int k=0 ; k<N ; k++) {
                    c[i*N+j] += a[i*N+k] * b[j*N+k];
                }
            }
        }
#endif
#ifdef BLOCKS
        for (int w=0 ; w<workers ; w++) {
            MPI_Recv(&b[w*N*blocksize], N*blocksize, MPI_DOUBLE, COORD_ID, TAG_DEFAULT, MPI_COMM_WORLD, &status);

            for (int i=0 ; i<blocksize ; i++) {
                for (int j=0 ; j<blocksize ; j++) {
                    for (int k=0 ; k<N ; k++) {
                        c[i*N+w*blocksize+j] += a[i*N+k] * b[j*N+w*blocksize*N+k];
                    }
                }
            }
        }
#endif

        MPI_Send(c, N*blocksize, MPI_DOUBLE, COORD_ID, TAG_DEFAULT, MPI_COMM_WORLD);
    }

    MPI_Finalize();

    free(a);
    free(b);
    free(c);
}
