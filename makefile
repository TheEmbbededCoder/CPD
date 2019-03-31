CC=gcc
CFLAGS= -fopenmp -I. -lm -Wall -std=gnu11 -g
DEPS = debug.h init_program.h physics.h
OBJ =  debug.c init_program.c physics.c simpar.c

default: simpar

all: simpar

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

simpar: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o simpar 