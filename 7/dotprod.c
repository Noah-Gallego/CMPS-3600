/* 
 * dotprod.c
 * 
 *  A threaded program to compute the dot product of two vectors. The
 *  algorithm is:
 *
 *          double sum = 0.0; 
 *          for (int i=0; i<N; i++) 
 *                sum += x[i] * y[i]; 
 *
 *  E.g., a = <2,3,4>  and  b = <-1,3,5>
 *  The dot product is: (2*-1)+(3*3)+(4*5) = -2+9+20 = 27.
 *
 *  A mutex enforces mutual exclusion on the shared variable 'sum'.
 *  Each thread must lock before updating and unlock after updating.
 *
 *  The main program creates threads which do all the work. After rejoining,
 *  the main thread prints out the final result. Before creating the threads,
 *  main initialize the shared data structures. Main then creates the threads
 *  and simply waits for all threads to complete. 
 *
 *  We specify a thread attribute value that allows the main thread to join with
 *  the threads it creates. Note also that we free up handles when they are
 *  no longer needed.
 * 
 *  Each thread works on a different set of data.  The offset is specified by 
 *  the loop variable 'i'. The size of the data for each thread is indicated by 
 *  VECLEN.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
	double      *a;
	double      *b;
	double     sum; 
	int     veclen; 
} DOTDATA;

#define NUMTHRDS 4
#define VECLEN 100
DOTDATA dotstr;
pthread_t callThd[NUMTHRDS];
pthread_mutex_t mutexsum;     /* a mutex to protect the dot product */


void *dotprod(void *arg)
{
	/* thread function */
	int i, start, end, len;
	long offset;
	double mysum, *x, *y;
	offset = (long)arg;

	len = dotstr.veclen;
	start = offset * len;
	end   = start + len;
	x = dotstr.a;
	y = dotstr.b;

	if (offset == 2)
		printf("this is thread #2\n");

	mysum = 0;
	for (i=start; i<end; i++) {
		mysum += (x[i] * y[i]);
	}
	pthread_mutex_lock (&mutexsum);
	dotstr.sum += mysum;
	pthread_mutex_unlock (&mutexsum);
	pthread_exit((void*) 0);
}


int main (int argc, char *argv[])
{
	long i;
	double *a, *b;
	double val;
	void *status;
	pthread_attr_t attr;

	/* Assign storage and initialize values in the vectors */
	a = (double *)malloc(NUMTHRDS * VECLEN * sizeof(double));
	b = (double *)malloc(NUMTHRDS * VECLEN * sizeof(double));
	val = 1.0;
	for (i=0; i<VECLEN * NUMTHRDS; i++) {
		a[i] = val;
		b[i] = a[i];
		val += 0.011;
	}

	dotstr.veclen = VECLEN;
	dotstr.a = a;
	dotstr.b = b;
	dotstr.sum = 0;

	pthread_mutex_init(&mutexsum, NULL);

	/* Create threads to perform the dotproduct */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	for (i=0; i<NUMTHRDS; i++) {
		pthread_create(&callThd[i], &attr, dotprod, (void *)i);
	}

	pthread_attr_destroy(&attr);

	/* Wait on the other threads */
	for(i=0; i<NUMTHRDS; i++) {
		pthread_join(callThd[i], &status);
	}

	/* After joining, print out the results and cleanup */
	printf("All threads have completed their work.\n");
	printf("dot product of vectors: %f\n", dotstr.sum);
	free(a);
	free(b);
	pthread_mutex_destroy(&mutexsum);
	pthread_exit(NULL);
}


