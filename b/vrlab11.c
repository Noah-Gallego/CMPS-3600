#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

typedef struct {
	int *a;
	int *b;
	/* int total; */
	int veclen;
} Dotdata;

#define NUM_THRDS 8
#define VEC_LEN 132

Dotdata dotstr;               /* global so all threads can see and use it */
pthread_t callThd[NUM_THRDS];
pthread_mutex_t mutexsum;     /* use a mutex to protect the dot product */
int pipefd[2];
int count;

void *dotprod(void *arg)
{
	/* thread function */
	int i, start, end, len ;
	long offset;
	int localsum, *x, *y;
	offset = (long)arg;

	len = dotstr.veclen;
	start = offset * len;
	end   = start + len;
	x = dotstr.a;
	y = dotstr.b;

	localsum = 0;
	for (i=start; i<end; i++) {
		localsum += (x[i] * y[i]);
	}
	
	pthread_mutex_lock (&mutexsum);
	/* dotstr.total += localsum; */
	if (count > 1) printf("LocalSum: %*d\n", 10, localsum);
	write(pipefd[1], &localsum, 4);

	pthread_mutex_unlock (&mutexsum);
	pthread_exit((void *)0);
}



int main (int argc, char *argv[]) {
	long i;
	int *a, *b;
	void *status;
	int in;
	pid_t pid;
	int stat;

	if (pipe(pipefd) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}

	/* Assign storage and initialize values in the vectors */
	count = argc;
	a = (int *)malloc(NUM_THRDS * VEC_LEN * sizeof(int));
	b = (int *)malloc(NUM_THRDS * VEC_LEN * sizeof(int));
	for (i=0; i<VEC_LEN * NUM_THRDS; i++) {
		a[i] = 1.0;
		b[i] = a[i];  /* over written in the next statement */
		b[i] = (i+1); /* integers from 1 to VECLEN*NUMTHRDS*/
	}

	dotstr.veclen = VEC_LEN; 
	dotstr.a = a; 
	dotstr.b = b; 

	pid = fork();

	if (pid == 0) { /* Child Process */
		int total = 0;

		/* Declare Local Variable + Close Read End Of Pipe */
		close(pipefd[1]);

		/* Read */
		while (read(pipefd[0], &in, sizeof(int)) != 0)
			total += in;

		/* Print Total */
		if (count > 1) printf("%*s\n", 20, "--------");
		printf("The Dot Product Sum: %d\n", total);
		

		wait(&stat);
		if (WIFEXITED(stat)) {
			printf("Child process exited with status: %d\n", WEXITSTATUS(stat));
		}

		/* Close Read Pipe */
		close(pipefd[0]);
		_exit(EXIT_SUCCESS);
	} else { /* Parent Process */
		close(pipefd[0]);

		pthread_mutex_init(&mutexsum, NULL);

		/* create NUMTHRDS threads */
		for (i=0; i<NUM_THRDS; i++) {
			pthread_create(&callThd[i], NULL, dotprod, (void *)i);
		}

		/* wait on all the threads to finish */
		for (i=0; i<NUM_THRDS; i++) {
			pthread_join(callThd[i], &status);
		}

		close(pipefd[1]);
	}

	if (a) free(a);
	if (b) free(b);
	pthread_mutex_destroy(&mutexsum);

	return 0;
}