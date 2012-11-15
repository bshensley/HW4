#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <omp.h>

/* Prints grid to screen, with (0,0) in upper left */
int printgrid(double **grid, const int nx) {
    for (int j = 0; j < nx; j++) {
      for (int i = 0; i < nx; i++) {
	printf("%f ", grid[i][j]);
      }
      printf("\n");
    }
    return 0;
}


int main(int argc, char *argv[]) {
  const int nx = atoi(argv[1]);
  const int nthreads = atoi(argv[2]);
  const int chunk = nx / nthreads;

  /* Equation parameters, constants */
  const double kappa = 1.0;
  const double pi = 3.14159265358979;
  const double tmax = 0.5*pi*pi/kappa;
  const double dx = pi / (nx-1);
  const double dt = 0.5*dx*dx/(4.0*kappa);

  /* Initialize grid to nx by nx grid */ 
  double **curr = (double**)malloc(nx * sizeof(double));
  double **prev = (double**)malloc(nx * sizeof(double));
  assert(curr != NULL && prev != NULL);


  for (int i = 0; i < nx; i++) {
    curr[i] = (double*)malloc(nx * sizeof(double));
    prev[i] = (double*)malloc(nx * sizeof(double));
    assert(curr[i] != NULL && prev[i] != NULL);
  }
    #pragma omp parallel shared(prev, curr)
  {
        #pragma omp for schedule(dynamic,chunk) nowait
    for (int i = 0; i < nx; i++) {
      curr[i][0] = pow(cos(dx*i),2);
      prev[i][0] = pow(cos(dx*i),2);
      curr[i][nx-1] = pow(sin(dx*i),2);
      prev[i][nx-1] = pow(sin(dx*i),2);
      for (int j = 1; j < nx - 1; j++) {
	curr[i][j] = 0.0;
	prev[i][j] = 0.0;
      }
    }
  }

  /* Implement finite difference method */
  for (double t = 0; t < tmax; t += dt) {
        #pragma omp parallel shared(prev, curr)
    {
      #pragma omp for schedule(dynamic,chunk) nowait
      for (int i = 0; i < nx; i++) {
	for (int j = 1; j < nx - 1; j++) {
	  if (i == 0) {
	    prev[i][j] = curr[i][j] + 
	      dt*kappa*(curr[nx-2][j] + curr[i+1][j] + 
			curr[i][j-1] + curr[i][j+1] - 
			4.0*curr[i][j])/(dx*dx);
	  }
	  else if (i == nx - 1) {
	    prev[i][j] = prev[0][j];
	  }
	  else {
	    prev[i][j] = curr[i][j] + 
	      dt*kappa*(curr[i-1][j] + curr[i+1][j] + 
			curr[i][j-1] + curr[i][j+1] - 
			4.0*curr[i][j])/(dx*dx);
	  }
	}
      }
      /* Swap the current and previous grids */
      double **tmp = curr;
      curr = prev;
      prev = tmp;
    }
  }

  /* Compute the average temperature */
  double avg = 0;
#pragma omp parallel shared(prev, curr, avg)
  {
#pragma omp for schedule(dynamic,chunk) reduction(+:avg) nowait
    for (int i = 0; i < nx; i++) {
      for (int j = 0; j < nx; j++) {
	avg = curr[i][j];
      }
    }
  }
  avg = avg/(nx*nx);
  printf("#%lf\n", avg);

  /* Final data dump */
  printgrid(curr, nx);

  /* Free arrays */
  for (int i = 0; i < nx; i++) {
    free(curr[i]);
    free(prev[i]);
  }
  free(prev);
  free(curr);
}
