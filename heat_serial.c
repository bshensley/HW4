#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>

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
  clock_t begin, end;
  double exec_time;
  begin = clock();

  const int nx = atoi(argv[1]);
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
    curr[i][0] = pow(cos(dx*i),2);
    prev[i][0] = pow(cos(dx*i),2);
    curr[i][nx-1] = pow(sin(dx*i),2);
    prev[i][nx-1] = pow(sin(dx*i),2);
    for (int j = 1; j < nx - 1; j++) {
      curr[i][j] = 0.0;
      prev[i][j] = 0.0;
    }
  }

  /* Implement finite difference method */
  for (double t = 0; t < tmax; t += dt) {
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

  /* Compute the average temperature */
  double avg = 0;
  for (int i = 0; i < nx; i++) {
    for (int j = 0; j < nx; j++) {
      avg += curr[i][j];
    }
  }
  avg = avg/(nx*nx);

  /* Final data dump */
  printf("#%lf\n", avg);
  printgrid(curr, nx);

  /* Free arrays */
  for (int i = 0; i < nx; i++) {
    free(curr[i]);
    free(prev[i]);
  }
  free(prev);
  free(curr);

  end = clock();
  exec_time = (double)(end - begin) / CLOCKS_PER_SEC;
  printf("#%lf\n", exec_time);
}
  

