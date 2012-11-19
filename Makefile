CC = icc
CCMPI = mpicc
OBJS_SERIAL := heat_serial.o
OBJS_OMP := heat_omp.o
OBJS_MPI := heat_mpi.o
OBJS := $(OBJS_SERIAL) $(OBJS_OMP)

CFLAGS = -Wall -std=c99 -O3

all : heat_serial heat_omp heat_mpi

$(OBJS_SERIAL): CFLAGS := -Wall -g -O3 -std=c99
$(OBJS_OMP): CFLAGS := -fopenmp -Wall -O3 -std=c99
$(OBJS_MPI): CFLAGS := -Wall -O3 -std=c99

heat_serial : $(OBJS_SERIAL)
	$(CC) -o $@ $^

heat_omp : $(OBJS_OMP)
	$(CC) -fopenmp -o $@ $^

heat_mpi : $(OBJS_MPI)
	$(CCMPI) -o $@ $^

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJS_MPI): %.o: %.c
	$(CCMPI) -c $(CFLAGS) $< -o $@

clean :
	rm -rf *o heat_serial heat_omp heat_mpi