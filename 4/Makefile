# Makefile for week-4

all: shared_mem shm_io sigset sigset2 strings vrlab4

shared_mem: shared_mem.c
	gcc shared_mem.c -Wall -o shared_mem

shm_io: shm_io.c
	gcc shm_io.c -Wall -o shm_io

sigset: sigset.c
	gcc sigset.c -Wall -o sigset

sigset2: sigset2.c
	gcc sigset2.c -Wall -o sigset2

strings: strings.c
	gcc strings.c -Wall -o strings

vrlab4: vrlab4.c
	gcc vrlab4.c -Wall -o vrlab4

clean:
	rm -f shared_mem shm_io sigset sigset2 strings vrlab4
