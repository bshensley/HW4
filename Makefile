CC = gcc
CFLAGS = -Wall -std=c99

all : heat_serial heat_omp

heat_serial : heat_serial.o
	$(CC) $(CFLAGS) -g -o heat_serial heat_serial.o

heat_omp : heat_omp.o
	$(CC) $(CFLAGS) -fopenmp -o heat_omp heat_omp.o

clean :
	rm -rf *o heat_serial heat_omp