MCC=mpicc
CC=gcc

CFLAGS = -O2 -Wall
TARGET = pmap_dist pmap pmap_index

all : $(TARGET)

clean: 
	-rm -f *.o
	-rm -f $(TARGET)

CCFLAGS=$(CFLAGS)

pmap_dist: pmap_dist.c mpi_util.c Makefile
	$(MCC) $(CFLAGS) pmap_dist.c -o pmap_dist -lz

pmap: pmap.c mpi_util.c Makefile
	$(MCC) $(CFLAGS) pmap.c -o pmap -lz

pmap_index: pmap_index.c Makefile
	$(CC) $(CFLAGS) pmap_index.c -o pmap_index -lz

