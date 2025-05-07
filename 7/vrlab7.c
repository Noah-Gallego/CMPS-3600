// Noah Gallego

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sem.h> // System V Semaphore
#include <sys/ipc.h>
#include <sys/types.h>

// Gloabal Variables
int done = 0;
int eat[5] = {0, 0, 0, 0, 0};
int max_eats = 100;
int fib_n = 5;
int semid;

// Union
union {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
} my_semun;

// Semaphore Array Of Eats
struct sembuf grab[5][3], release[5][3], wait_for_zero; // Grab Release Array

// Prototypes
void *philosopher(void*);
int fib(int);
void setup_grab_release_arrays(void);

void *philosopher(void *arg) {
    int id = (int)(long)arg;

    while (!done) {
        // Think 🧠
        if (id == 0) semop(semid, &wait_for_zero, 1);

        // Critical Seciton Start (Eat) 🍽️
        semop(semid, grab[id], 3);

        // Calculate nth Fib Number (For Delay)
        fib(fib_n);
        eat[id]++;

        printf("%i eating %i %i %i %i %i \n", id, eat[0], eat[1], eat[2], eat[3], eat[4]);

        semop(semid, release[id], 3);
        // Critical Section End (Eat) 🍽️

        // Rest 😴
        if (eat[id] > max_eats) done = 1;
    }

    return (void *)0;
}

int main(int argc, char *argv[]) {
    // Step Two: Command Line Arguments
    if (argc > 1) max_eats = atoi(argv[1]);
    if (argc > 2) fib_n = atoi(argv[2]);

    pthread_t threadID[5];

    /* Set Up Semaphores */
    semid = semget(IPC_PRIVATE, 6, IPC_CREAT | 0666);

    // Initialize All Forks to 1
    for (int i = 0; i < 5; i++) {
        my_semun.val = 1;
        semctl(semid, i, SETVAL, my_semun.val);
    }

    // Mutex Semaphore
    my_semun.val = 1; 
    semctl(semid, 5, SETVAL, my_semun.val);

    setup_grab_release_arrays();

    // Create Philosopher Threads
    for (int i = 0; i < 5; i++) {
        pthread_create(&threadID[i], NULL, philosopher, (void *)(long)i);
    }

    // Join Threads
    for (int i = 0; i < 5; i++) {
        pthread_join(threadID[i], NULL);
    }

    // Clean Up IPC
    semctl(semid, 0, IPC_RMID, 0);

    return 0;
}

// Step One: Add A Fib Function
int fib(int n) {
    return (n == 1 || n == 2) ? 1 : (fib(n - 1) + fib(n - 2));
}

// Initilaize Semaphores
void setup_grab_release_arrays() {
    for (int i = 0; i < 5; i++) {
        int left = i;
        int right = (i + 1) % 5;

        grab[i][0].sem_num = 5;
        grab[i][0].sem_op = -1;
        grab[i][0].sem_flg = SEM_UNDO;

        // Fork Grabbing
        grab[i][1].sem_num = left;
        grab[i][1].sem_op = -1;
        grab[i][1].sem_flg = SEM_UNDO;

        grab[i][2].sem_num = right;
        grab[i][2].sem_op = -1;
        grab[i][2].sem_flg = SEM_UNDO;

        // Fork Releasing
        release[i][0].sem_num = left;
        release[i][0].sem_op = 1;
        release[i][0].sem_flg = SEM_UNDO;

        release[i][1].sem_num = right;
        release[i][1].sem_op = 1;
        release[i][1].sem_flg = SEM_UNDO;

        release[i][2].sem_num = 5;
        release[i][2].sem_op = 1;  
        release[i][2].sem_flg = SEM_UNDO;
    }

    // Wait For Zero
    wait_for_zero.sem_num = 5; // Mutex Semaphore
    wait_for_zero.sem_op = 0;
    wait_for_zero.sem_flg = SEM_UNDO;
}