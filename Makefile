CC = gcc
CFLAGS = -O2 -g -Wall -std=c99

all : heat_serial

heat_serial : heat_serial.o
	$(CC) $(CFLAGS) -o heat_serial heat_serial.o

clean :
	rm -rf *o heat_serial