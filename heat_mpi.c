#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "mpi.h"

int main(int argc, char *argv[]) {
  clock_t begin, end;
  double exec_time;
  begin = clock();

  int numtasks, rank;
  MPI_Request reqs[4];
  MPI_Status stats[4];
  int rc = MPI_Init(&argc,&argv);
  if (rc != MPI_SUCCESS) {
    printf ("Error starting MPI program. Terminating.\n");
    MPI_Abort(MPI_COMM_WORLD, rc);
  }

  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const int nx = atoi(argv[1]);
  const int len = nx / numtasks;
  //  int start = rank * (nx / numtasks);
  //  int end = (rank + 1) * (nx / numtasks) - 1;

  /* Equation parameters, constants */
  const double kappa = 1.0;
  const double pi = 3.14159265358979;
  const double tmax = 0.5*pi*pi/kappa;
  const double dx = pi / (nx-1);
  const double dt = 0.5*dx*dx/(4.0*kappa);

  /* Initialize grid for each process, including ghost cells
     Domain is chopped into nx/numtasks strips, with two
     columns of ghost cells on either side */
  double **curr = (double**)malloc((len+2) * sizeof(double));
  double **prev = (double**)malloc((len+2) * sizeof(double));
  assert(curr != NULL && prev != NULL);
  for (int i = 0; i < len + 2; i++) {
    curr[i] = (double*)malloc(nx * sizeof(double));
    prev[i] = (double*)malloc(nx * sizeof(double));
    assert(curr[i] != NULL && prev[i] != NULL);
  }

  for (int i = 0; i < len + 2; i++) {
    double xcoord = dx*(rank * len + i - 1);
    curr[i][0] = pow(cos(xcoord),2);
    prev[i][0] = pow(cos(xcoord),2);
    curr[i][nx-1] = pow(sin(xcoord),2);
    prev[i][nx-1] = pow(sin(xcoord),2);
    for (int j = 1; j < nx - 1; j++) {
      curr[i][j] = 0.0;
      prev[i][j] = 0.0;
    }
  }

  /* Implement finite difference method */
  for (double t = 0; t < tmax; t += dt) {
    int rprev, rnext;
    rprev = rank - 1;
    if (rank == 0) {
      rprev = numtasks - 1;
    }
    rnext = rank + 1;
    if (rank == numtasks - 1) {
      rnext = 0;
    }

    /* Receive data from adjacent blocks and place into ghost cells */
    MPI_Irecv(&curr[len+1][0], nx, MPI_DOUBLE, rnext, 1, MPI_COMM_WORLD, &reqs[0]);
    MPI_Irecv(&curr[0][0], nx, MPI_DOUBLE, rprev, 2, MPI_COMM_WORLD, &reqs[1]);

    /* Pass data to ghost cells of adjacent blocks */
    if (rank == numtasks - 1) {
      MPI_Isend(&curr[len-1][0], nx, MPI_DOUBLE, rnext, 2, MPI_COMM_WORLD, &reqs[3]);
    }
    else {
      MPI_Isend(&curr[len][0], nx, MPI_DOUBLE, rnext, 2, MPI_COMM_WORLD, &reqs[3]);
    }
    if (rank == 0) {
      MPI_Isend(&curr[2][0], nx, MPI_DOUBLE, rprev, 1, MPI_COMM_WORLD, &reqs[2]);
    }
    else {
      MPI_Isend(&curr[1][0], nx, MPI_DOUBLE, rprev, 1, MPI_COMM_WORLD, &reqs[2]);
    }

    MPI_Waitall(4, reqs, stats);
    
    for (int i = 1; i < len + 1; i++) {
      for (int j = 1; j < nx - 1; j++) {
	prev[i][j] = curr[i][j] + 
	  dt*kappa*(curr[i-1][j] + curr[i+1][j] + 
		    curr[i][j-1] + curr[i][j+1] - 
		    4.0*curr[i][j])/(dx*dx);
      }
    }

    /* Swap the current and previous grids */
    double **tmp = curr;
    curr = prev;
    prev = tmp;
  }

  /* Compute the average temperature */
  double sum = 0.0;
  for (int i = 1; i < len + 1; i++) {
    for (int j = 0; j < nx; j++) {
      sum += curr[i][j];
    }
  }
  double sum_tot;
  MPI_Allreduce(&sum, &sum_tot, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

  /* Final data dump */
  if (rank == 0) {
    double avg = sum_tot/(nx*nx);
    printf("#%lf\n", avg);
  }

  char format[] = "mpi.%d.%d.%d.out";
  char fname[sizeof format + 100];
  sprintf(fname,format,nx,rank,numtasks);
  FILE *fp;
  fp = fopen(fname, "w");
  for (int j = 0; j < nx; j++) {
    for (int i = 1; i < len + 1; i++) {
      fprintf(fp, "%f ", curr[i][j]);
    }
    fprintf(fp, "\n");
  }

  /* Free arrays */
  for (int i = 0; i < len + 2; i++) {
    free(curr[i]);
    free(prev[i]);
  }
  free(prev);
  free(curr);

  MPI_Finalize();

  end = clock();
  exec_time = (double)(end - begin) / CLOCKS_PER_SEC;
  if (rank == 0) {
    printf("#%lf\n", exec_time);
  }
}
