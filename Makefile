CC = gcc
OBJS_SERIAL := heat_serial.o
OBJS_OMP := heat_omp.o
OBJS := $(OBJS_SERIAL) $(OBJS_OMP)

CFLAGS = -Wall -std=c99 -O3

all : heat_serial heat_omp

$(OBJS_SERIAL): CFLAGS := -Wall -g -O3 -std=c99
$(OBJS_OMP): CFLAGS := -fopenmp -Wall -O3 -std=c99

heat_serial : $(OBJS_SERIAL)
	$(CC) -o $@ $^

heat_omp : $(OBJS_OMP)
	$(CC) -fopenmp -o $@ $^

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	rm -rf *o heat_serial heat_omp